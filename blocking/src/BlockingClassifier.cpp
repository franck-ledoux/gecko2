/*----------------------------------------------------------------------------*/
#include <gecko/blocking/BlockingClassifier.h>
#include <gmds/utils/Exception.h>
/*----------------------------------------------------------------------------*/
#include "gecko/blocking/Graph.h"
#include <limits>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
/*----------------------------------------------------------------------------*/
using namespace gecko;
using namespace gecko::blocking;
/*----------------------------------------------------------------------------*/
BlockingClassifier::BlockingClassifier(std::shared_ptr<Blocking> ABlocking) : m_blocking(ABlocking), m_geom_model(ABlocking->geom_model()) {}
/*----------------------------------------------------------------------------*/
BlockingClassifier::~BlockingClassifier() {}
/*----------------------------------------------------------------------------*/
ClassificationErrors
BlockingClassifier::detect_classification_errors()
{
	ClassificationErrors errors;
	// 1) We check the geometric issues first
	// The number of block nodes is greater than the number of geom points
	// When a point is captured by a node, we do not need to check this node again for another point, no?

	std::vector<cad::GeomPoint *> geom_points;
	m_geom_model->getPoints(geom_points);

	std::vector<cad::GeomCurve *> geom_curves;
	m_geom_model->getCurves(geom_curves);

	for (auto p : geom_points) {

		auto [found, n] = find_node_classified_on(p);
		if (!found) {
			errors.non_captured_points.push_back(p->id());
		}
	}
	// 2) We check the geometric curves
	// Again the number of edges is greater than the number of curves.
	//========================================================================
	// 1. We get all the edges and nodes of the block structure that are
	// classified on curves, and we associated them to the corresponding curve
	// id
	//========================================================================
	std::map<int, std::vector<Blocking::Edge>> edgesOnCurve;
	std::map<int, std::vector<Blocking::Node>> nodesOnCurve;

	for (auto nId : m_blocking->mesh().nodes()) {
		auto n = m_blocking->mesh().get<Node>(nId);
		if (m_blocking->get_geom_dim(n)== cad::GeomMeshLinker::LinkCurve) {
			nodesOnCurve[m_blocking->get_geom_id(n)].push_back(n);
		}
	}
	for (auto eId : m_blocking->mesh().edges()) {
		auto e = m_blocking->mesh().get<Edge>(eId);
		if (m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::LinkCurve) {
			edgesOnCurve[m_blocking->get_geom_id(e)].push_back(e);
		}
	}

	//========================================================================
	// 2. Now we check the capture of the curve. We consider 3 cases:
	// - the curve has two distinct end points
	// - the curve has only one end point
	// - the curve is a cycle without end points
	//========================================================================

	for (auto c : geom_curves) {
		auto end_points = c->points();

		std::pair<bool, std::vector<Blocking::Edge>> c_info;

		if (end_points.size() == 2) {
			c_info = find_edges_classified_on_curve_with_2_end_points(c, nodesOnCurve[c->id()], edgesOnCurve[c->id()]);
		}
	/*	else if (end_points.size() == 1) {

			c_info = find_edges_classified_on_curve_with_1_end_point(c, nodesOnCurve[c->id()], edgesOnCurve[c->id()]);
		}
		else if (end_points.size() == 0) {

			c_info = find_edges_classified_on_curve_without_end_points(c, nodesOnCurve[c->id()], edgesOnCurve[c->id()]);
		}*/
		else
			throw GMDSException("A curve cannot have more than two end points");

		if (c_info.first == false) errors.non_captured_curves.push_back(c->id());
	}

	//========================================================================
	// 3) We check the geometric surfaces
	std::vector<cad::GeomSurface *> geom_surfaces;
	m_geom_model->getSurfaces(geom_surfaces);
	if (!errors.non_captured_points.empty() || !errors.non_captured_curves.empty()){
		//we capture surfaces only if points and curves are captured
		for(auto s:geom_surfaces)
			errors.non_captured_surfaces.push_back(s->id());
	}
	else{
		//curves and surfaces are captured, we can try and capture surfaces
		for (auto s : geom_surfaces) {
			auto [found, fs] = find_faces_classified_on(s);
			// if all points and all curves are not captured, we don't class the faces so, the surfaces are not captured
			if(!found)
				errors.non_captured_surfaces.push_back(s->id());
		}
	}


	//========================================================================
	// 3) We check the geometric volume

	return errors;
}
/*----------------------------------------------------------------------------*/
void
BlockingClassifier::clear_classification()
{
	//Clear classification of the nodes
	for (auto nId : m_blocking->mesh().nodes()) {
		auto n = m_blocking->mesh().get<Node>(nId);
		m_blocking->set_geom_link(n,cad::GeomMeshLinker::NoLink,NullID);
	}
	//Clear classification of the edges
	for (auto eId : m_blocking->mesh().edges()) {
		auto e = m_blocking->mesh().get<Edge>(eId);
		m_blocking->set_geom_link(e,cad::GeomMeshLinker::NoLink,NullID);
	}
	//Clear classification of the faces
	for (auto fId : m_blocking->mesh().faces()) {
		auto f = m_blocking->mesh().get<Face>(fId);
		m_blocking->set_geom_link(f,cad::GeomMeshLinker::NoLink,NullID);
	}
	for (auto rId : m_blocking->mesh().regions()) {
		auto r = m_blocking->mesh().get<Region>(rId);
		m_blocking->set_geom_link(r,cad::GeomMeshLinker::NoLink,NullID);
	}
}
/*----------------------------------------------------------------------------*/
int
BlockingClassifier::try_and_classify_nodes(std::set<TCellID> &ANodeIds, const double ATolerance)
{
	// We get all the geometric entities of the model
	std::vector<cad::GeomPoint *> geom_points;
	std::vector<cad::GeomCurve *> geom_curves;
	std::vector<cad::GeomSurface *> geom_surfaces;
	m_geom_model->getPoints(geom_points);
	m_geom_model->getCurves(geom_curves);
	m_geom_model->getSurfaces(geom_surfaces);

	int nb_unclassified = 0;
	double epsilon = 10e-5;
	// initial projection stage
	for (auto &current_node_id : ANodeIds) {
		auto current_node = m_blocking->mesh().get<Node>(current_node_id);
		math::Point p = current_node.point();
		// We first check if the node is already classified on a point

		if (m_blocking->get_geom_dim(current_node) == cad::GeomMeshLinker::LinkPoint)
			continue;


		std::vector<cad::GeomEntity *> cells;
		cells.insert(cells.end(), geom_points.begin(), geom_points.end());
		auto [closest_pnt_dist, closest_pnt_id, closest_pnt_loc] = get_closest_cell(p, cells);
		cells.clear();
		cells.insert(cells.end(), geom_curves.begin(), geom_curves.end());
		auto [closest_curv_dist, closest_curv_id, closest_curv_loc] = get_closest_cell(p, cells);
		cells.clear();
		cells.insert(cells.end(), geom_surfaces.begin(), geom_surfaces.end());
		auto [closest_surf_dist, closest_surf_id, closest_surf_loc] = get_closest_cell(p, cells);

		if (closest_pnt_dist - closest_curv_dist <= epsilon &&
		    closest_pnt_dist - closest_surf_dist <= epsilon &&
		    closest_pnt_dist <= ATolerance) {     // On point

			m_blocking->set_geom_link(current_node,cad::GeomMeshLinker::LinkPoint,closest_pnt_id);
			// projection is done next line
			current_node.setPoint(closest_pnt_loc);
		}
		else if (closest_curv_dist - closest_pnt_dist  < epsilon &&
		         closest_curv_dist - closest_surf_dist <= epsilon &&
		         closest_curv_dist <= ATolerance) {     // On curve

			m_blocking->set_geom_link(current_node,cad::GeomMeshLinker::LinkCurve,closest_curv_id);
			// projection is done next line
			current_node.setPoint(closest_curv_loc);
		}
		else if (closest_surf_dist - closest_pnt_dist  < epsilon &&
		         closest_surf_dist - closest_curv_dist  < epsilon &&
		         closest_surf_dist <= ATolerance) {     // On surface

			m_blocking->set_geom_link(current_node,cad::GeomMeshLinker::LinkSurface,closest_surf_id);
			// projection is done next line
			current_node.setPoint(closest_surf_loc);
		}
		else {
			// the node is not classified and keep is location
			m_blocking->set_geom_link(current_node,cad::GeomMeshLinker::NoLink,NullID);
			nb_unclassified++;
		}
	}
	return nb_unclassified;
}
/*----------------------------------------------------------------------------*/
std::pair<bool, Blocking::Edge>
BlockingClassifier::find_aligned_edge(cad::GeomPoint *APoint, const math::Vector3d &ATangent, std::set<TCellID> &AEdgeIds)
{
	std::vector<Blocking::Edge> candidates;
	for (auto eid : AEdgeIds) {
		auto e = m_blocking->mesh().get<Edge>(eid);
		auto e_nodes = e.get<Node>();
		if (( m_blocking->get_geom_dim(e_nodes[0]) == cad::GeomMeshLinker::LinkPoint && ( m_blocking->get_geom_id(e_nodes[0]) == APoint->id()))
		    || ( m_blocking->get_geom_dim(e_nodes[1]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(e_nodes[1]) == APoint->id())) {
			candidates.push_back(e);
		}
	}
	// Among the candidates, does one aligned with tangent0?
	bool found_aligned = false;
	Blocking::Edge aligned_edge;
	for (auto i = 0; i < candidates.size() && !found_aligned; i++) {
		auto c = candidates[i];
		auto c_nodes = c.get<Node>();
		// We go from 1 to 0
		auto c_vec = c_nodes[0].point() - c_nodes[1].point();
		if (m_blocking->get_geom_dim(c_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(c_nodes[0]) == APoint->id()) {
			// means we have to go from 0 to 1
			c_vec = c_nodes[1].point() - c_nodes[0].point();
		}
		c_vec.normalize();
		if (c_vec.dot(ATangent) > 0.7) {
			found_aligned = true;
			aligned_edge = c;
		}
	}
	return std::make_pair(found_aligned, aligned_edge);
}
/*----------------------------------------------------------------------------*/
bool BlockingClassifier::alreadyClass(std::vector<TCellID> listElements)
{

	for (auto i = 0; i < listElements.size(); i++) {
		auto n_id = listElements[i];
		if (i > 0) {
			auto m_id = listElements[i - 1];
			auto e_mn = m_blocking->get_edge(n_id, m_id);
			if (m_blocking->get_geom_dim(e_mn)== cad::GeomMeshLinker::LinkCurve) {
				return true;
			}
		}
	}
	return false;
}
/*----------------------------------------------------------------------------*/
int
BlockingClassifier::try_and_capture(std::set<TCellID> &ANodeIds,
                                    std::set<TCellID> &AEdgeIds,
                                    std::set<TCellID> &AFaceIds)
{
	//===================================================================
	// 1. WE CHECK NODE
	//===================================================================
	try_and_classify_nodes(ANodeIds);

	std::vector<std::pair<int, Blocking::Node>> classNodes;
	for (auto nId : ANodeIds) {
		auto n = m_blocking->mesh().get<Node>(nId);
		classNodes.push_back(std::make_pair(m_blocking->get_geom_dim(n),n));
	}

	//===================================================================
	// 2. WE WORK ON CURVES
	//===================================================================
	// We get  geometric entities of the model
	std::vector<cad::GeomCurve *> geom_curves;
	std::vector<cad::GeomSurface *> geom_surfaces;
	m_geom_model->getCurves(geom_curves);
	m_geom_model->getSurfaces(geom_surfaces);

	std::map<TCellID ,bool> is_captured_curves;
	std::map<TCellID ,bool> is_captured_surfaces;
	std::map<size_t,std::vector<std::pair<int,std::vector<TCellID>>>> captCurvesMap;


	// PAS CONTINU
	// sortir fonction creation graph en prenant entree seulement ANodesIds et AEdgesIds
	Graph g(ANodeIds);
	for (auto i : AEdgeIds) {
		auto ei_nodes = m_blocking->mesh().get<Edge>(i).get<Node>();
		g.addEdge(ei_nodes[0].id(), ei_nodes[1].id(), 1);
	}

	for (auto c : geom_curves) {
		is_captured_curves[c->id()] = false;
		// 3 cases based on the number of end points of c (0,1,2)
		auto c_end_points = c->points();
		if (c_end_points.size() == 0) {
			throw GMDSException("Capture of curves with 0 end points is not yet supported");
		}
		else if (c_end_points.size() == 1) {
			throw GMDSException("Capture of curves with 1 end point is not yet supported");
		}
		if (c_end_points.size() == 2) {
			auto end_point_0 = c_end_points[0];
			auto end_point_1 = c_end_points[1];
			// We check if those end points are already captured?
			bool found_n0 = false, found_n1 = false;
			Blocking::Node n0, n1;
			for (auto id_node : ANodeIds) {
				auto ni = m_blocking->mesh().get<Node>(id_node);
				if (m_blocking->get_geom_dim(ni) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(ni) == end_point_0->id()) {
					found_n0 = true;
					n0 = ni;
				}
				else if (m_blocking->get_geom_dim(ni) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(ni) ==  end_point_1->id()) {
					found_n1 = true;
					n1 = ni;
				}
			}
			if (found_n0 && found_n1) {
				// The two end points of the curve are already captured, we can look for a path going
				// from n0 ton n1 among the nodes of ANodeIDs
				// We look for each node, if a block edge is aligned enough with the curve

				// ============ END POINT 0 first =======================
				auto info0 = find_aligned_edge(end_point_0, c->tangent(0), AEdgeIds);
				auto info1 = find_aligned_edge(end_point_1, c->tangent(1), AEdgeIds);
				if (info0.first && info1.first) {
					// means we found two edge tangential to the curve at its extremities
					// We can try to capture the curve

					// First case;, the edges founded for each node are in fact the same
					auto first_edge = info0.second;
					auto second_edge = info1.second;
					if (m_blocking->get_geom_dim(first_edge) == cad::GeomMeshLinker::LinkCurve
						|| m_blocking->get_geom_dim(second_edge) == cad::GeomMeshLinker::LinkCurve){
						is_captured_curves[c->id()] = true;
						continue;
					}
					else if (first_edge == second_edge) {
						// same one, we assign it to the curve
						m_blocking->set_geom_link(first_edge,cad::GeomMeshLinker::LinkCurve,c->id());
						is_captured_curves[c->id()]= true;
					}
					// second case, we check if the two edges have a common node, it is over
					else {
						auto found_common_node = false;
						Blocking::Node common_node;
						auto first_nodes = m_blocking->mesh().get<Edge>(first_edge.id()).get<Node>();
						auto second_nodes = m_blocking->mesh().get<Edge>(second_edge.id()).get<Node>();
						if ((first_nodes[0] == second_nodes[0]) || first_nodes[0] == second_nodes[1]) {
							found_common_node = true;
							common_node = first_nodes[0];
						}
						else if ((first_nodes[1] == second_nodes[0]) || first_nodes[1] == second_nodes[1]) {
							found_common_node = true;
							common_node = first_nodes[1];
						}
						if (found_common_node) {
							m_blocking->set_geom_link(common_node,cad::GeomMeshLinker::LinkCurve,c->id());
							gmds::math::Point tempPoint = common_node.point();
							c->project(tempPoint);
							m_blocking->set_geom_link(first_edge,cad::GeomMeshLinker::LinkCurve,c->id());
							m_blocking->set_geom_link(second_edge,cad::GeomMeshLinker::LinkCurve,c->id());
							is_captured_curves[c->id()]= true;
						}
						else {
							//Third case, We need the shortest path that connect the end points of our two edges
							auto src_node = first_nodes[0];
							if (m_blocking->get_geom_dim(first_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(first_nodes[0]) == end_point_0->id()) src_node = first_nodes[1];
							auto dest_node = second_nodes[0];
							if (m_blocking->get_geom_dim(second_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(second_nodes[0]) == end_point_1->id()) dest_node = second_nodes[1];

							/* for (auto eid : AEdgeIds) {
							      auto e = m_blocking->get_edge(eid);
							      if (e == first_edge || e == second_edge) continue;
							      auto e_nodes = m_blocking->get_nodes_of_edge(e);
							      auto e_middle = 0.5 * (e_nodes[0]->info().point + e_nodes[1]->info().point);
							      auto p_on_curve = e_middle;
							      c->project(p_on_curve);
								   auto wi = e_middle.distance(p_on_curve)+1;
							    //  g.setWeight(e_nodes[0]->info().topo_id, e_nodes[1]->info().topo_id, wi);
							   }*/
							g.computeDijkstra(src_node.id());
							auto all_path = g.getShortestPath();
							std::vector<TCellID> sp = all_path[dest_node.id()];
							double spw = g.getShortestPathWeights()[dest_node.id()];

							auto average_w = spw / sp.size();
							if (average_w < 1000) {
								// arbitrary value to avoid to classify wrong paths
								//captCurvesMap.insert({sp.size(),{c->id(),sp}});
								captCurvesMap[sp.size()].push_back({c->id(),sp});
							}
						}
					}
				}
			}
		}
	}

	// for(auto mapElmt : captCurvesMap){
	// 	if(!alreadyClass(mapElmt.second.second)){
	// 		m_blocking->save_vtk_blocking("temp_save_debug");
	//
	// 		auto c = m_geom_model->getCurve(mapElmt.second.first);
	// 		auto c_end_points = c->points();
	// 		auto end_point_0 = c_end_points[0];
	// 		auto end_point_1 = c_end_points[1];
	// 		auto info0 = find_aligned_edge(end_point_0, c->tangent(0), AEdgeIds);
	// 		auto info1 = find_aligned_edge(end_point_1, c->tangent(1), AEdgeIds);
	// 		auto first_edge = info0.second;
	// 		auto second_edge = info1.second;
	//
	// 		m_blocking->set_geom_link(first_edge,cad::GeomMeshLinker::LinkCurve,c->id());
	// 		m_blocking->set_geom_link(second_edge,cad::GeomMeshLinker::LinkCurve,c->id());
	//
	// 		for (auto i = 0; i < mapElmt.second.second.size(); i++) {
	// 			auto n_id = mapElmt.second.second[i];
	// 			auto n = m_blocking->mesh().get<Node>(n_id);
	// 			m_blocking->set_geom_link(n,cad::GeomMeshLinker::LinkCurve,c->id());
	// 			auto nPoint = n.point();
	// 			c->project(nPoint);
	//
	// 			if (i > 0) {
	// 				auto m_id = mapElmt.second.second[i - 1];
	// 				auto e_mn = m_blocking->get_edge(n_id, m_id);
	// 				m_blocking->set_geom_link(e_mn,cad::GeomMeshLinker::LinkCurve,c->id());
	// 			}
	// 		}
	//
	// 		is_captured_curves[c->id()]= true;
	// 	}
	// }
	for (const auto& mapElmt : captCurvesMap) {
		const auto& pairs = mapElmt.second;
		for (const auto& curveInfo : pairs) {
			int curve_id = curveInfo.first;
			const std::vector<TCellID>& chemin = curveInfo.second;

			if (!alreadyClass(chemin)) {
				auto c = m_geom_model->getCurve(curve_id);
				auto c_end_points = c->points();
				auto end_point_0 = c_end_points[0];
				auto end_point_1 = c_end_points[1];
				auto info0 = find_aligned_edge(end_point_0, c->tangent(0), AEdgeIds);
				auto info1 = find_aligned_edge(end_point_1, c->tangent(1), AEdgeIds);
				auto first_edge = info0.second;
				auto second_edge = info1.second;

				m_blocking->set_geom_link(first_edge, cad::GeomMeshLinker::LinkCurve, c->id());
				m_blocking->set_geom_link(second_edge, cad::GeomMeshLinker::LinkCurve, c->id());

				for (size_t i = 0; i < chemin.size(); ++i) {
					auto n_id = chemin[i];
					auto n = m_blocking->mesh().get<Node>(n_id);
					m_blocking->set_geom_link(n, cad::GeomMeshLinker::LinkCurve, c->id());

					auto nPoint = n.point();
					c->project(nPoint);

					if (i > 0) {
						auto m_id = chemin[i - 1];
						auto e_mn = m_blocking->get_edge(n_id, m_id);
						m_blocking->set_geom_link(e_mn, cad::GeomMeshLinker::LinkCurve, c->id());
					}
				}

				is_captured_curves[c->id()] = true;
			}
		}
	}

	//===================================================================
	// 3. WE WORK ON SURFACES
	//===================================================================
	//We try and capture surfaces only if all the curves are captured. It makes
	// the surface capture algorithm easier to write
	bool found_one_uncaptured_curve = false;

	for(auto c :geom_curves){
		if (!is_captured_curves[c->id()])
			found_one_uncaptured_curve = true;
	}
	if(!found_one_uncaptured_curve){
		//we can try and capture surfaces
		//we store in a map the color of boundary block faces

		//we use the boundary faces only, AFaceIds vector is only bnd faces
		std::vector<Blocking::Face> bnd_faces;
		bnd_faces.reserve(AFaceIds.size());
		for(auto f_id : AFaceIds) {
			bnd_faces.push_back(m_blocking->mesh().get<Face>(f_id));
		}
		auto map_faces_colored = blocking_color_faces(bnd_faces);

		// We class the faces with a surface
 		for (auto s : geom_surfaces) {

			auto s_points = s->points();
			auto s_curves = s->curves();
			int color_of_this_surface = -1;

			for (auto f : map_faces_colored) {
				int nb_edges_on_curve = 0;
				int nb_nodes_on_point = 0;

				auto nodes_f = m_blocking->mesh().get<Face>(f.first.id()).get<Node>();
				auto edges_f = m_blocking->mesh().get<Face>(f.first.id()).get<Edge>();
				// We check if the face has 1 node classified on a point and 2 edges class on curves
				for (auto n : nodes_f) {
					if (m_blocking->get_geom_dim(n) == cad::GeomMeshLinker::LinkPoint) {
						for (auto p : s_points) {
							if (p->id() == m_blocking->get_geom_id(n)) {
								nb_nodes_on_point++;
							}
						}
					}
				}
				for (auto e : edges_f) {
					if (m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::LinkCurve) {
						for (auto c : s_curves) {
							if (c->id() == m_blocking->get_geom_id(e)) {
								nb_edges_on_curve++;
							}
						}
					}
				}

				if (nb_nodes_on_point >= 1 && nb_edges_on_curve >= 2) {
					color_of_this_surface = f.second;
				}
			}
			for (auto f : map_faces_colored) {
				// We do nothing if the color of the face is 0, and any edges is class on a curve,
				// because the face is not on the boundary, but its poss
				if (f.second == 0) {
					auto face = f.first;
					m_blocking->set_geom_link(face,cad::GeomMeshLinker::NoLink,NullID);
				}
				else if (f.second == color_of_this_surface) {
						auto face = f.first;
						m_blocking->set_geom_link(face,cad::GeomMeshLinker::LinkSurface,s->id());

					// We classify the element of the face not class on the surface
					auto nodes_f = m_blocking->mesh().get<Face>(f.first.id()).get<Node>();
					auto edges_f = m_blocking->mesh().get<Face>(f.first.id()).get<Edge>();
					// First, the nodes
					for (auto n : nodes_f) {
						if (m_blocking->get_geom_dim(n) == cad::GeomMeshLinker::NoLink) {
							m_blocking->set_geom_link(n,cad::GeomMeshLinker::LinkSurface,s->id());
							auto nPoint = n.point();
							s->project(nPoint);
						}
					}
					// And the edges
					for (auto e : edges_f) {
						if (m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::NoLink|| m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::LinkVolume) {
							m_blocking->set_geom_link(e,cad::GeomMeshLinker::LinkSurface,s->id());
						}
					}
					is_captured_surfaces[s->id()]= true;
				}
			}
		}
	}

	//===================================================================
	// 4. WE WORK ON VOLUMES
	//===================================================================
	//We try and capture volumes only if all the surfaces are captured. It makes
	// the volume capture algorithm easier to write
	bool found_one_uncaptured_surface = false;
	auto geom_volumes = m_geom_model->getVolumes();

	for(auto s :geom_surfaces){
		if (!is_captured_surfaces[s->id()])
			found_one_uncaptured_surface= true;
	}
	if(!found_one_uncaptured_surface) {
		// Mono volumic classification
		if (geom_volumes.size() == 1) {
			auto v = geom_volumes[0];
			if (m_blocking->is_valid_connected()) {
				for (auto nId : m_blocking->mesh().nodes()) {
					auto n = m_blocking->mesh().get<Node>(nId);
					if (m_blocking->get_geom_dim(n) == cad::GeomMeshLinker::NoLink) {
						m_blocking->set_geom_link(n,cad::GeomMeshLinker::LinkVolume,v->id());
					}
				}
				for (auto eId : m_blocking->mesh().edges()) {
					auto e = m_blocking->mesh().get<Edge>(eId);
					if (m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::NoLink) {
						m_blocking->set_geom_link(e,cad::GeomMeshLinker::LinkVolume,v->id());
					}
				}
				for (auto fId : m_blocking->mesh().faces()) {
					auto f = m_blocking->mesh().get<Face>(fId);
					if (m_blocking->get_geom_dim(f) == cad::GeomMeshLinker::NoLink) {
						m_blocking->set_geom_link(f,cad::GeomMeshLinker::LinkVolume,v->id());
					}
				}
				for (auto bId :m_blocking->mesh().regions()) {
					auto b = m_blocking->mesh().get<Region>(bId);
					if (m_blocking->get_geom_dim(b) == cad::GeomMeshLinker::NoLink) {
						m_blocking->set_geom_link(b,cad::GeomMeshLinker::LinkVolume,v->id());
					}
				}
			}
		}
	}


	return 0;
}
/*----------------------------------------------------------------------------*/
bool
BlockingClassifier::build_edge_path_for_curve(cad::GeomCurve *AC,
                                              cad::GeomPoint *AP1,
                                              cad::GeomPoint *AP2,
                                              Blocking::Node &AN1,
                                              Blocking::Node &AN2,
                                              std::vector<TCellID> &ANodeIDs,
                                              std::vector<TCellID> &AEdgeIDs)
{
	return false;
}
/*----------------------------------------------------------------------------*/
//
// void
// BlockingClassifier::classify_nodes(ClassificationErrors &AErrors, const double AMaxDistance, const double APointSnapDistance)
// {
// 	CMap3 *gm = m_blocking->cmap();
// 	std::vector<cad::GeomPoint *> geom_points;
// 	std::vector<cad::GeomCurve *> geom_curves;
// 	std::vector<cad::GeomSurface *> geom_surfaces;
// 	m_geom_model->getPoints(geom_points);
// 	m_geom_model->getCurves(geom_curves);
// 	m_geom_model->getSurfaces(geom_surfaces);
//
// 	// initial projection stage
// 	for (auto &current_node : gm->attributes<0>()) {
// 		math::Point p = current_node.info().point;
//
// 		std::vector<cad::GeomEntity *> cells;
// 		cells.insert(cells.end(), geom_points.begin(), geom_points.end());
// 		auto [closest_pnt_dist, closest_pnt_id, closest_pnt_loc] = get_closest_cell(p, cells);
// 		cells.clear();
// 		cells.insert(cells.end(), geom_curves.begin(), geom_curves.end());
// 		auto [closest_curv_dist, closest_curv_id, closest_curv_loc] = get_closest_cell(p, cells);
// 		cells.clear();
// 		cells.insert(cells.end(), geom_surfaces.begin(), geom_surfaces.end());
// 		auto [closest_surf_dist, closest_surf_id, closest_surf_loc] = get_closest_cell(p, cells);
//
// 		if (closest_pnt_dist <= closest_curv_dist && closest_pnt_dist <= closest_surf_dist && closest_pnt_dist <= AMaxDistance) {     // On point
// 			current_node.info().geom_dim = 0;
// 			current_node.info().geom_id = closest_pnt_id;
// 			// projection is done next line
// 			current_node.info().point = closest_pnt_loc;
// 		}
// 		else if (closest_curv_dist < closest_pnt_dist && closest_curv_dist <= closest_surf_dist && closest_curv_dist <= AMaxDistance) {     // On curve
// 			current_node.info().geom_dim = 1;
// 			current_node.info().geom_id = closest_curv_id;
// 			// projection is done next line
// 			current_node.info().point = closest_curv_loc;
// 		}
// 		else if (closest_surf_dist < closest_pnt_dist && closest_surf_dist < closest_curv_dist && closest_surf_dist <= AMaxDistance) {     // On surface
// 			current_node.info().geom_dim = 2;
// 			current_node.info().geom_id = closest_surf_id;
// 			// projection is done next line
// 			current_node.info().point = closest_surf_loc;
// 		}
// 		else {
// 			// the node is not classified and keep is location
// 			current_node.info().geom_dim = 4;
// 			current_node.info().geom_id = NullID;
// 			AErrors.non_classified_nodes.push_back(current_node.info().topo_id);
// 		}
// 	}
// 	// snapping to the closest point if mandatory
// 	for (auto &current_node : gm->attributes<0>()) {
// 		// we only consider nodes that are not classified on points
// 		if (current_node.info().geom_dim != 0) {
// 			math::Point p = current_node.info().point;
// 			std::vector<cad::GeomEntity *> cells;
// 			cells.insert(cells.end(), geom_points.begin(), geom_points.end());
// 			auto [closest_pnt_dist, closest_pnt_id, closest_pnt_loc] = get_closest_cell(p, cells);
// 			if (closest_pnt_dist < APointSnapDistance) {     // On point
// 				current_node.info().geom_dim = 0;
// 				current_node.info().geom_id = closest_pnt_id;
// 				// projection is done next line
// 				current_node.info().point = closest_pnt_loc;
// 			}
// 		}
// 	}
// }
// /*----------------------------------------------------------------------------*/
// void
// BlockingClassifier::classify_edges(::ClassificationErrors &AErrors)
// {
// 	CMap3 *gm = m_blocking->cmap();
// 	std::vector<cad::GeomPoint *> geom_points;
// 	std::vector<cad::GeomCurve *> geom_curves;
// 	std::vector<cad::GeomSurface *> geom_surfaces;
// 	m_geom_model->getPoints(geom_points);
// 	m_geom_model->getCurves(geom_curves);
// 	m_geom_model->getSurfaces(geom_surfaces);
//
// 	// initial projection stage
// 	for (auto it = gm->attributes<1>().begin(), itend = gm->attributes<1>().end(); it != itend; ++it) {
// 		std::vector<Blocking::Node> ending_nodes = m_blocking->get_nodes_of_edge(it);
// 		auto geo_d0 = ending_nodes[0]->info().geom_dim;
// 		auto geo_d1 = ending_nodes[1]->info().geom_dim;
// 		auto geo_i0 = ending_nodes[0]->info().geom_id;
// 		auto geo_i1 = ending_nodes[1]->info().geom_id;
//
// 		/* We list possible configuration of ending nodes classification:
// 		 * We decide to not classify an edge on a surface during this process.
// 		 * IMPORTANT: We'll classify the edges in this case, when we'll classify the faces
// 		 * 1) Nodes are on different geom points. If those points have a common curve, then the
// 		 *    edge is on this curve. If they have several common curves,we don't know.
// 		 * 1bis) We decide to don't
// 		 * 2) Nodes are on different geom points. If those points have no common curve, but a
// 		 *    common surface, then the edge is on the surface.
// 		 * 3) Nodes are on the same curve, then is the edge.
// 		 * 4) One node is on a point P and the other on a curve, then the edge is
// 		 *    on this curve if it is adjacent to point P
// 		 * 5) Otherwise we don't know how to classify the edge
// 		 * 6) One node n0 is on a point P and an edge e0 part of the node is almost tangent of a curve, \\TODO
// 		 * we check if the other node n1 of the edge is unclassified. If that is the case, we classify
// 		 * the edge e0 and the node n1 on the curve
// 		 * 7) Close to the configuration 6, but the first node is on a curve. We can use the same principle \\TODO
// 		 */
// 		if (geo_d0 == 0 && geo_d1 == 0 && geo_i0 != geo_i1) {
// 			// We look for a common curve
// 			if ((geo_i0 == 2 && geo_i1 == 4) || (geo_i0 == 4 && geo_i1 == 2)) {
// 				std::cout << "Cas à tester" << std::endl;
// 			}
// 			cad::GeomPoint *p0 = m_geom_model->getPoint(geo_i0);
// 			cad::GeomPoint *p1 = m_geom_model->getPoint(geo_i1);
// 			auto curve_id = m_geom_model->getCommonCurve(p0, p1);
// 			if (curve_id != -1) {
// 				//				//We need to check if the classification is available. Check the angle between the curve take and the edge
// 				//				//Compute the unit vector of the curve
// 				//				int AParam = 0;
// 				//				auto vectorTang = m_geom_model->getCurve(curve_id)->computeTangent(AParam);
// 				//
// 				//				if(AParam == 0){
// 				//					std::cout<<"check vect : "<<vectorTang<<std::endl;
// 				//				}
// 				//
// 				//				//Do the same for the edge take
// 				//				auto vectEdge = ending_nodes[1]->info().point-ending_nodes[0]->info().point;
// 				//
// 				//				std::cout<<"check vect edge : "<<vectEdge<<std::endl;
// 				//
// 				//				//Compute angle between the 2 vectors
// 				//
// 				//				double scalarProduct = vectorTang.dot(vectEdge.normalize());
// 				//
// 				//				std::cout<<"check scalar product : "<<scalarProduct<<std::endl;
// 				//
// 				//				//check if the scalar product is under [-0.5,0.5]
// 				//
// 				//				if(scalarProduct<=0.5 && scalarProduct>=-0.5){
// 				//					// Nothing (CONFIGURATION 1bis)
// 				//					it->info().geom_dim = 4;
// 				//					it->info().geom_id = NullID;
// 				//					AErrors.non_classified_edges.push_back(it->info().topo_id);
// 				//				}
// 				//				else{
// 				//					//We have a common curve (CONFIGURATION 1)
// 				//					it->info().geom_dim = 1;
// 				//					it->info().geom_id = curve_id;
// 				//				}
//
// 				// We have a common curve (CONFIGURATION 1)
// 				it->info().geom_dim = 1;
// 				it->info().geom_id = curve_id;
// 			}
// 			else {
// 				// Nothing (CONFIGURATION 5)
// 				it->info().geom_dim = 4;
// 				it->info().geom_id = NullID;
// 				AErrors.non_classified_edges.push_back(it->info().topo_id);
// 			}
// 		}
// 		else if (geo_d0 == 1 && geo_d1 == 1 && geo_i0 == geo_i1) {
// 			// On the same curve (CONFIGURATION 3)
// 			it->info().geom_dim = 1;
// 			it->info().geom_id = geo_i0;
// 		}
// 		else if ((geo_d0 == 0 && geo_d1 == 1) || (geo_d0 == 1 && geo_d1 == 0)) {
// 			// we check if the point is adjacent to the curve
// 			auto p_id = (geo_d0 < geo_d1) ? geo_i0 : geo_i1;
// 			auto c_id = (geo_d0 > geo_d1) ? geo_i0 : geo_i1;
// 			cad::GeomPoint *p = m_geom_model->getPoint(p_id);
// 			cad::GeomCurve *c = m_geom_model->getCurve(c_id);
// 			std::vector<cad::GeomPoint *> c_points = c->points();
// 			if (c_points[0] == p || c_points[1] == p) {
// 				// On a curve (CONFIGURATION 4)
// 				it->info().geom_dim = 1;
// 				it->info().geom_id = c_id;
// 			}
// 		}
// 		else {
// 			// Nothing (CONFIGURATION 5)
// 			it->info().geom_dim = 4;
// 			it->info().geom_id = NullID;
// 			AErrors.non_classified_edges.push_back(it->info().topo_id);
// 		}
// 	}
//
// 	// Now that edges are clasified on curves
// }
// /*----------------------------------------------------------------------------*/
// void
// BlockingClassifier::classify_faces(::ClassificationErrors &AErrors)
// {
// 	CMap3 *gm = m_blocking->cmap();
// 	std::vector<cad::GeomPoint *> geom_points;
// 	std::vector<cad::GeomCurve *> geom_curves;
// 	std::vector<cad::GeomSurface *> geom_surfaces;
// 	m_geom_model->getPoints(geom_points);
// 	m_geom_model->getCurves(geom_curves);
// 	m_geom_model->getSurfaces(geom_surfaces);
//
// 	// check if all points and all curves are captured, we can classify the faces
// 	if (AErrors.non_captured_curves.empty() && AErrors.non_captured_points.empty()) {
// 		auto faces = m_blocking->get_all_faces();
// 		auto map_faces_colored = blocking_color_faces();
//
// 		// We class the faces with a surface
// 		for (auto s : geom_surfaces) {
//
// 			std::vector<cad::GeomPoint *> s_points = s->points();
// 			std::vector<cad::GeomCurve *> s_curves = s->curves();
// 			int color_of_this_surface = -1;
//
// 			for (auto f : map_faces_colored) {
// 				int nb_edges_on_curve = 0;
// 				int nb_nodes_on_point = 0;
//
// 				auto nodes_f = m_blocking->get_nodes_of_face(f.first);
// 				auto edges_f = m_blocking->get_edges_of_face(f.first);
// 				// We check if the face have 1 node class on a point and 2 edges class on curves
// 				for (auto n : nodes_f) {
// 					if (n->info().geom_dim == 0) {
// 						for (auto p : s_points) {
// 							if (p->id() == n->info().geom_id) {
// 								nb_nodes_on_point++;
// 							}
// 						}
// 					}
// 				}
// 				for (auto e : edges_f) {
// 					if (e->info().geom_dim == 1 && m_blocking->get_faces_of_edge(e).size() == 2) {
// 						for (auto c : s_curves) {
// 							if (c->id() == e->info().geom_id) {
// 								nb_edges_on_curve++;
// 							}
// 						}
// 					}
// 				}
//
// 				if (nb_nodes_on_point >= 1 && nb_edges_on_curve >= 2) {
// 					color_of_this_surface = f.second;
// 				}
// 			}
// 			for (auto f : map_faces_colored) {
// 				// We do nothing if the color of the face is 0, and any edges is class on a curve, because the face is not on the boundary, but its poss
// 				if (f.second == 0) {
// 					f.first->info().geom_id = -1;
// 					f.first->info().geom_dim = 4;
// 				}
//
// 				else if (f.second == color_of_this_surface) {
// 					if (f.first->info().geom_dim == 4) {
// 						f.first->info().geom_id = s->id();
// 						f.first->info().geom_dim = s->dim();
// 					}
// 					// We classify the element of the face not class on the surface
// 					auto nodes_f = m_blocking->get_nodes_of_face(f.first);
// 					auto edges_f = m_blocking->get_edges_of_face(f.first);
// 					// First, the nodes
// 					for (auto n : nodes_f) {
// 						if (n->info().geom_dim == 4) {
// 							n->info().geom_dim = s->dim();
// 							n->info().geom_id = s->id();
// 						}
// 					}
// 					// And the edges
// 					for (auto e : edges_f) {
// 						if (e->info().geom_dim == 4) {
// 							e->info().geom_dim = s->dim();
// 							e->info().geom_id = s->id();
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// 	auto nodes = m_blocking->get_all_nodes();
// 	AErrors.non_classified_nodes.clear();
// 	for (auto n : nodes) {
// 		if (n->info().geom_dim == 4) {
// 			AErrors.non_classified_nodes.push_back(n->info().topo_id);
// 		}
// 	}
//
// 	AErrors.non_classified_edges.clear();
//
// 	auto edges = m_blocking->get_all_edges();
// 	for (auto e : edges) {
// 		if (e->info().geom_dim == 4) {
// 			AErrors.non_classified_edges.push_back(e->info().topo_id);
// 		}
// 	}
// 	AErrors.non_classified_faces.clear();
//
// 	auto faces = m_blocking->get_all_faces();
// 	for (auto f : faces) {
// 		if (f->info().geom_dim == 4) {
// 			AErrors.non_classified_faces.push_back(f->info().topo_id);
// 		}
// 	}
// }
// /*----------------------------------------------------------------------------*/
// ClassificationErrors
// BlockingClassifier::classify(const double AMaxDistance, const double APointSnapDistance)
// {
// 	ClassificationErrors errors;
// 	//============ (1) We classify nodes =================
// 	classify_nodes(errors, AMaxDistance, APointSnapDistance);
//
// 	//============ (2) We classify edges =================
// 	classify_edges(errors);
//
// 	errors = detect_classification_errors();
//
// 	if (errors.non_captured_points.empty() && errors.non_captured_curves.empty()) {
//
// 		//============ (2) We classify faces =================
// 		classify_faces(errors);
//
// 		errors.non_captured_surfaces.clear();
//
// 		errors = detect_classification_errors();
// 	}
// 	else {
// 		errors.non_classified_nodes.clear();
// 		for (auto n : m_blocking->get_all_nodes()) {
// 			if (n->info().geom_dim == 4) {
// 				errors.non_classified_nodes.push_back(n->info().topo_id);
// 			}
// 		}
// 		errors.non_classified_edges.clear();
// 		for (auto e : m_blocking->get_all_edges()) {
// 			if (e->info().geom_dim == 4) {
// 				errors.non_classified_edges.push_back(e->info().topo_id);
// 			}
// 		}
// 		for (auto f : m_blocking->get_all_faces()) {
// 			errors.non_classified_faces.push_back(f->info().topo_id);
// 		}
// 	}
//
// 	return errors;
// }
//
/*----------------------------------------------------------------------------*/
std::pair<bool, Blocking::Node>
BlockingClassifier::find_node_classified_on(cad::GeomPoint *AP)
{
	for (auto nId : m_blocking->mesh().nodes()) {
		auto n = m_blocking->mesh().get<Node>(nId);
		if (m_blocking->get_geom_dim(n) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(n) == AP->id()) return {true, n};
	}
	auto nId = *(m_blocking->mesh().nodes().begin());
	return {false, m_blocking->mesh().get<Node>(nId)};
}

/*----------------------------------------------------------------------------*/
std::pair<bool, std::vector<Blocking::Edge>>
BlockingClassifier::find_edges_classified_on_curve_with_2_end_points(cad::GeomCurve *AC,
                                                                     std::vector<Blocking::Node> &ANodesOnAC,
                                                                     std::vector<Blocking::Edge> &AEdgesOnAC)
{
	// this method is called from BlockingClassifier::find_edges_classified_on_curve
	// and must have two end points

	//Hypothesis on input parameters:
	//- We know that all the nodes and edges are classified on AC
	//What we have to do:
	// - check that end points of AC are extremities of edges in AEdgesOnAC
	//- every node of ANodesOnAC is in two edges of AEdgesOnAc
	assert(AC->points().size() == 2);

	auto p1 = AC->points()[0];
	auto p2 = AC->points()[1];

	bool found_p1 = false;
	bool found_p2 = false;
	std::map<int, int> nb_edges_adj_to_node;
	for(auto e: AEdgesOnAC){
		auto e_nodes = m_blocking->mesh().get<Edge>(e.id()).get<Node>();
		if(!found_p1){
			//We check if the current edge knows p1
			if ((m_blocking->get_geom_dim(e_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(e_nodes[0]) == p1->id() ) ||
			    ( m_blocking->get_geom_dim(e_nodes[1]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(e_nodes[1]) == p1->id()))
				found_p1 = true;
		}
		if(!found_p2){
			//We check if the current edge knows p1
			if ((m_blocking->get_geom_dim(e_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(e_nodes[0])== p2->id() ) ||
			    ( m_blocking->get_geom_dim(e_nodes[1]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(e_nodes[1]) == p2->id()))
				found_p2 = true;
		}

		nb_edges_adj_to_node[e_nodes[0].id()]++;
		nb_edges_adj_to_node[e_nodes[1].id()]++;
	}
	if(!found_p1 || !found_p2)
		return std::make_pair(false, AEdgesOnAC);

	for(auto n : ANodesOnAC){
		if (nb_edges_adj_to_node[n.id()]!=2)
			return std::make_pair(false, AEdgesOnAC);
	}

	return std::make_pair(true, AEdgesOnAC);

}
/*----------------------------------------------------------------------------*/
std::pair<bool, std::vector<Blocking::Edge>>
BlockingClassifier::find_edges_classified_on_curve_with_1_end_point(cad::GeomCurve *AC,
                                                                    std::vector<Blocking::Node> &ANodesOnAC,
                                                                    std::vector<Blocking::Edge> &AEdgesOnAC)
{
	// this method is called from BlockingClassifier::find_edges_classified_on_curve
	// and must have one end points
	assert(AC->points().size() == 1);
	auto p1 = AC->points()[0];

	if (AEdgesOnAC.size() == 1) {
		// We consider this case is an error
		return std::make_pair(false, AEdgesOnAC);
	}

	// We are in the general case. We are going to start from p1 and check if the set
	// of edges that are in AEdgesOnAC discretizes AC. It means, we have to go from p1
	// to p1 using ALL the edges of AEdgesOnAC.
	bool found_first_edge = false;
	int first_edge_index = -1;
	for (auto index_e = 0; index_e < AEdgesOnAC.size() && !found_first_edge; index_e++) {
		Blocking::Edge ei = AEdgesOnAC[index_e];
		auto ei_nodes = m_blocking->mesh().get<Edge>(ei.id()).get<Node>();
		if ((m_blocking->get_geom_dim(ei_nodes[0]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(ei_nodes[0]) == p1->id())
		    || (m_blocking->get_geom_dim(ei_nodes[1]) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(ei_nodes[1]) == p1->id())) {
			found_first_edge = true;
			first_edge_index = index_e;
		}
	}

	if (!found_first_edge) return std::make_pair(false, AEdgesOnAC);

	// Ok here we have found the first edge, which is our current edge now
	auto current_edge = AEdgesOnAC[first_edge_index];
	// we look for the next edge until reaching point p2
	auto current_nodes = m_blocking->mesh().get<Edge>(current_edge.id()).get<Node>();
	Blocking::Node next_node = current_nodes[0];
	if (m_blocking->get_geom_id(next_node) == p1->id()) {
		// means the next node is in the second node of the edge
		next_node = current_nodes[1];
	}
	auto nb_traversed_edge = 1;     // the first one
	auto reach_p1_again = false, found_error = false;
	while (!reach_p1_again && !found_error) {
		// We look for the next edge
		bool found_next_edge = false;
		// We check that the node we use for finding the next edge belongs to ANodeAC
		auto found_node = false;
		for (auto n : ANodesOnAC) {
			if (next_node == n) found_node = true;
		}
		if (found_node == false) found_error = true;

		for (auto e_index = 0; e_index < AEdgesOnAC.size() && !found_next_edge; e_index++) {
			Blocking::Edge e = AEdgesOnAC[e_index];
			if (e == current_edge) continue;
			// we are not on the current edge.
			//  Does e is adjacent to current_node?
			auto e_nodes = m_blocking->mesh().get<Edge>(e.id()).get<Node>();
			if (e_nodes[0] == next_node) {
				next_node = e_nodes[1];
				found_next_edge = true;
				current_edge = e;
				nb_traversed_edge++;
			}
			else if (e_nodes[1] == next_node) {
				next_node = e_nodes[0];
				found_next_edge = true;
				current_edge = e;
				nb_traversed_edge++;
			}
		}
		if (!found_next_edge) found_error = true;
		if (m_blocking->get_geom_dim(next_node) == cad::GeomMeshLinker::LinkPoint && m_blocking->get_geom_id(next_node) == p1->id()) reach_p1_again = true;
	}

	if (reach_p1_again && nb_traversed_edge == AEdgesOnAC.size() && !found_error) return std::make_pair(true, AEdgesOnAC);

	// it means we have an error
	return std::make_pair(false, AEdgesOnAC);
}
/*----------------------------------------------------------------------------*/
std::pair<bool, std::vector<Blocking::Edge>>
BlockingClassifier::find_edges_classified_on_curve_without_end_points(cad::GeomCurve *AC,
                                                                      std::vector<Blocking::Node> &ANodesOnAC,
                                                                      std::vector<Blocking::Edge> &AEdgesOnAC)
{
	// this method is called from BlockingClassifier::find_edges_classified_on_curve
	// and must not have  end points
	assert(AC->points().size() == 0);

	// We pick a first edge and we  are going to start from its first node seed and check if the
	// set of edges that are in AEdgesOnAC discretizes AC. It means, we have to go from seed
	// to seed using ALL the edges of AEdgesOnAC.

	// Ok here we have found the first edge, which is our current edge now
	auto current_edge = AEdgesOnAC[0];
	// we look for the next edge until reaching point p2
	auto current_nodes = m_blocking->mesh().get<Edge>(current_edge.id()).get<Node>();
	Blocking::Node seed = current_nodes[0];
	Blocking::Node next_node = current_nodes[1];

	auto nb_traversed_edge = 1;     // the first one
	auto reach_seed_again = false, found_error = false;
	while (!reach_seed_again && !found_error) {
		// We look for the next edge
		bool found_next_edge = false;
		// We check that the node we use for finding the next edge belongs to ANodeAC
		auto found_node = false;
		for (auto n : ANodesOnAC) {
			if (next_node == n) found_node = true;
		}
		if (found_node == false) found_error = true;

		for (auto e_index = 0; e_index < AEdgesOnAC.size() && !found_next_edge; e_index++) {
			Blocking::Edge e = AEdgesOnAC[e_index];
			if (e == current_edge) continue;
			// we are not on the current edge.
			//  Does e is adjacent to current_node?
			auto e_nodes = m_blocking->mesh().get<Edge>(e.id()).get<Node>();
			if (e_nodes[0] == next_node) {
				next_node = e_nodes[1];
				found_next_edge = true;
				current_edge = e;
				nb_traversed_edge++;
			}
			else if (e_nodes[1] == next_node) {
				next_node = e_nodes[0];
				found_next_edge = true;
				current_edge = e;
				nb_traversed_edge++;
			}
		}
		if (!found_next_edge) found_error = true;
		if (next_node == seed) reach_seed_again = true;
	}

	if (reach_seed_again && nb_traversed_edge == AEdgesOnAC.size() && !found_error) return std::make_pair(true, AEdgesOnAC);

	// it means we have an error
	return std::make_pair(false, AEdgesOnAC);
}

/*----------------------------------------------------------------------------*/
std::pair<bool, std::vector<Blocking::Face>>
BlockingClassifier::find_faces_classified_on(cad::GeomSurface *AS)
{
	std::vector<Blocking::Face> facesOnSurface;

	for (auto fId : m_blocking->mesh().faces()) {
		auto f = m_blocking->mesh().get<Face>(fId);
		if (m_blocking->get_geom_dim(f) == cad::GeomMeshLinker::LinkSurface && m_blocking->get_geom_id(f) == AS->id()) {
			facesOnSurface.push_back(f);
		}
	}
	auto loopOfS = AS->loops();
	auto pointsOfS = AS->points();
	auto curvesOfS = AS->curves();
	std::set<int> listCurvesCapt;

	if (facesOnSurface.empty()) {
		return {false, facesOnSurface};
	}

	// We get all id curves capt by the edges of all the faces class on the surface
	for (auto f_S : facesOnSurface) {
		auto edges_F = m_blocking->mesh().get<Face>(f_S.id()).get<Edge>();
		for (auto e : edges_F) {
			if (m_blocking->get_geom_dim(e) == cad::GeomMeshLinker::LinkCurve) {
				listCurvesCapt.insert(m_blocking->get_geom_id(e));
			}
		}
	}
	for (auto l : loopOfS) {
		for (auto c : l) {
			if (listCurvesCapt.count(c->id()) == 0) {
				return {false, facesOnSurface};
			}
		}
	}
	return {true, facesOnSurface};
}
/*----------------------------------------------------------------------------*/
std::tuple<double, int, math::Point>
BlockingClassifier::get_closest_cell(const math::Point &AP, const std::vector<cad::GeomEntity *> &AGeomCells)
{
	math::Point closest_point = AGeomCells[0]->closestPoint(AP);
	double closest_distance = AP.distance(closest_point);
	int closest_id = AGeomCells[0]->id();
	for (auto geom_cell : AGeomCells) {
		math::Point current_point = geom_cell->closestPoint(AP);
		double current_dist = AP.distance(current_point);
		if (current_dist < closest_distance) {
			closest_distance = current_dist;
			closest_point = current_point;
			closest_id = geom_cell->id();
		}
	}
	return {closest_distance, closest_id, closest_point};
}

/*----------------------------------------------------------------------------*/
std::map<Blocking::Face, int>
BlockingClassifier::blocking_color_faces() {
	std::vector<Blocking::Face> bnd_faces;
	for (auto fId : m_blocking->mesh().faces()) {
		if (m_blocking->mesh().get<Face>(fId).get<Region>().size() == 1) {
			auto f = m_blocking->mesh().get<Face>(fId);
			bnd_faces.push_back(f);
		}
	}
	return blocking_color_faces(bnd_faces);
}
/*----------------------------------------------------------------------------*/
std::map<Blocking::Face, int>
BlockingClassifier::blocking_color_faces(const std::vector<Blocking::Face>& ABndFaces)
{
	std::map<Blocking::Face, int> colored_faces;
	auto mark_bnd  = m_blocking->mesh().newMark<Face>();
	auto mark_done = m_blocking->mesh().newMark<Face>();
	//we initialize faces as being not colored
	for (auto f : ABndFaces) {
		m_blocking->mesh().mark(f, mark_bnd);
	}
	int current_color = 0;
	bool finish = false;

	while (!finish) {
		finish = true;
		current_color++;
		Blocking::Face seed_face;
		bool found_seed = false;
		for (const auto& f : ABndFaces) {
			if (m_blocking->mesh().isMarked(f, mark_bnd) && !m_blocking->mesh().isMarked(f, mark_done)) {
				seed_face = f;
				found_seed = true;
				finish = false;
				break;
			}
		}
		if (found_seed) {
			//We have a set of faces to color
			std::set<Blocking::Face> front;
			front.insert(seed_face);
			while (!front.empty()) {
				auto current_face = *front.begin();
				front.erase(front.begin());
				colored_faces[current_face] = current_color;
				m_blocking->mesh().mark(current_face, mark_done);
				auto edges_f = m_blocking->mesh().get<Face>(current_face.id()).get<Edge>();
				for (auto e : edges_f) {
					// if the  edge e is not classified on a curve
					if (m_blocking->get_geom_dim(e) != cad::GeomMeshLinker::LinkCurve) {
						// We get adjacent faces
						auto e_faces = m_blocking->mesh().get<Edge>(e.id()).get<Face>();
						for (auto f : e_faces) {
							if (!m_blocking->mesh().isMarked(f,mark_done) &&
								m_blocking->mesh().isMarked(f,mark_bnd)){//m_blocking->mesh().get<Face>(f.id()).get<Region>().size() == 1) {
								front.insert(f);
								//m_blocking->mesh().mark(f, mark_done);
							}
						}
					}
				}
			}
		}
	}
	for (auto f : ABndFaces) {
		m_blocking->mesh().unmark(f, mark_bnd);
		m_blocking->mesh().unmark(f, mark_done);

	}
	m_blocking->mesh().freeMark<Face>(mark_bnd);
	m_blocking->mesh().freeMark<Face>(mark_done);
	return colored_faces;
}
/*----------------------------------------------------------------------------*/
bool
BlockingClassifier::checkValidity(ClassificationErrors &AErrors)
{
	if (m_blocking->mesh().getNbNodes() < m_geom_model->getNbPoints() || m_blocking->mesh().getNbEdges() < m_geom_model->getNbCurves()
	    || m_blocking->mesh().getNbFaces() < m_geom_model->getNbSurfaces() || m_blocking->mesh().getNbRegions() < m_geom_model->getNbVolumes()) {

		return false;
	}
	else if (AErrors.non_captured_points.size() != 0 || AErrors.non_captured_curves.size() != 0 || AErrors.non_captured_surfaces.size() != 0) {
		return false;
	}
	return true;
}

/*----------------------------------------------------------------------------*/
/*bool
BlockingClassifier::check_capt_element(const int AId, const int ADim)
{
   bool captPossible = false;
   auto listEdgesPara = m_blocking->get_all_sheet_edge_sets();
   auto allEdges = m_blocking->get_all_edges();
   if (ADim == 0) {
      if (check_cut_possible(AId, listEdgesPara)) {
         captPossible = true;
      }
   }
   else {
      auto theCurve = m_geom_model->getCurve(AId);
      gmds::TCoord minXYX[3];
      gmds::TCoord maxXYX[3];

      theCurve->computeBoundingBox(minXYX, maxXYX);

      gmds::math::Point minPoint(minXYX[0], minXYX[1], minXYX[2]);
      auto projMinPoint = m_blocking->get_projection_info(minPoint, allEdges);
      for (int i = 0; i < projMinPoint.size(); i++) {
         if (projMinPoint[i].second < 1 && projMinPoint[i].second > 0) {
            captPossible = true;
            break;
         }
      }

      gmds::math::Point maxPoint(maxXYX[0], maxXYX[1], maxXYX[2]);
      auto paramCutMaxPoint = m_blocking->get_cut_info(maxPoint);
      auto projMaxPoint = m_blocking->get_projection_info(maxPoint, allEdges);
      for (int i = 0; i < projMaxPoint.size(); i++) {
         if (projMaxPoint[i].second < 1 && projMaxPoint[i].second > 0) {
            captPossible = true;
            break;
         }
      }
   }
   return captPossible;
}*/
/*----------------------------------------------------------------------------*/
/*void
BlockingClassifier::capt_element(const int AnIdElement, const int ADim)
{
   auto listEdgesPara = m_blocking->get_all_sheet_edge_sets();

   if (ADim == 0) {
      auto paramCut = m_blocking->get_cut_info(AnIdElement);
      cut_sheet(std::get<0>(paramCut), std::get<1>(paramCut));
   }
   else {
      auto theCurve = m_geom_model->getCurve(AnIdElement);
      gmds::TCoord minXYX[3];
      gmds::TCoord maxXYX[3];

      theCurve->computeBoundingBox(minXYX, maxXYX);

      gmds::math::Point minPoint(minXYX[0], minXYX[1], minXYX[2]);

      auto paramCutMinPoint = m_blocking->get_cut_info(minPoint);

      gmds::math::Point maxPoint(maxXYX[0], maxXYX[1], maxXYX[2]);
      auto paramCutMaxPoint = m_blocking->get_cut_info(maxPoint);
   }
}*/
/*----------------------------------------------------------------------------*/
/*
bool
BlockingClassifier::check_cut_possible(int pointId, std::vector<std::vector<Blocking::Edge>> &AllEdges)
{
   bool cutPossible = false;

   //============================================
   auto noCaptPoint0 = m_geom_model->getPoint(pointId);
   gmds::math::Point p(noCaptPoint0->X(), noCaptPoint0->Y(), noCaptPoint0->Z());

   auto listEdgesPara = m_blocking->get_all_sheet_edge_sets();

   for (auto edges : listEdgesPara) {
      auto projInfo = m_blocking->get_projection_info(p, edges);
      for (int i = 0; i < projInfo.size(); i++) {
         if (projInfo[i].second < 1 && projInfo[i].second > 0) {
            cutPossible = true;
         }
      }
   }
   return cutPossible;
}*/
void BlockingClassifier::write(std::string filename) {
    m_blocking->save_vtk_blocking(filename);
}
/*----------------------------------------------------------------------------*/
