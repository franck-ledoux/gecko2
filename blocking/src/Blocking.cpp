/*----------------------------------------------------------------------------*/
#include "gecko/blocking//Blocking.h"

#include <boost/math/constants/constants.hpp>
#include <CGAL/boost/graph/graph_traits_HalfedgeDS_default.h>
#include <gmds/io/IGMeshIOService.h>
#include <gmds/io/VTKWriter.h>

#include "gmds/utils/LocalCellTopology.h"
/*----------------------------------------------------------------------------*/
using namespace gecko;
using namespace gecko::blocking;
using namespace gmds;
/*----------------------------------------------------------------------------*/
// int CellInfo::m_counter_global_id = 0;
/*----------------------------------------------------------------------------*/
Blocking::Blocking(cad::GeomManager *AGeomModel, bool AInitAsBoundingBox)
: m_geom_model(AGeomModel),
m_mesh(MeshModel(DIM3|R|F|E|N|
								   R2N|R2F|R2E|
								   F2N|F2R|F2E|
								   E2F|E2N|N2E|N2R)),
m_mesh_linker(&m_mesh,m_geom_model)
{
	if (AInitAsBoundingBox) {
		init_from_bounding_box();
	}
}

/*----------------------------------------------------------------------------*/
Blocking::Blocking(const Blocking &ABl) : m_geom_model(ABl.m_geom_model), m_mesh(ABl.m_mesh),
m_mesh_linker(ABl.m_mesh_linker) {

}
/*----------------------------------------------------------------------------*/
void Blocking::reset_classification() {
	for (auto n_id:m_mesh.nodes())
		m_mesh_linker.unlinkNode(n_id);
	for (auto e_id:m_mesh.edges())
		m_mesh_linker.unlinkEdge(e_id);
	for (auto f_id:m_mesh.faces())
		m_mesh_linker.unlinkFace(f_id);
}
/*----------------------------------------------------------------------------*/
Blocking::~Blocking() {
}
/*----------------------------------------------------------------------------*/
bool Blocking::operator==(Blocking &ABlocking) {
	if (m_mesh.getNbRegions()!=ABlocking.m_mesh.getNbRegions()) {
		return false;
	}
	else if (m_mesh.getNbFaces()!=ABlocking.m_mesh.getNbFaces()) {
		return false;
	}
	else if (m_mesh.getNbEdges()!=ABlocking.m_mesh.getNbEdges()) {
		return false;
	}
	else if (m_mesh.getNbNodes()!=ABlocking.m_mesh.getNbNodes()) {
		return false;
	}

	//we have the same number of 0, 1, 2 and 3- cells in both blockings
	bool sameBlocking = true;

	for (auto n1: m_mesh.nodes()) {
		bool found = false;
		math::Point p1 = m_mesh.get<Node>(n1).point();
		for (auto n2: ABlocking.mesh().nodes()) {

			math::Point p2 = ABlocking.mesh().get<Node>(n2).point();
			if (p1.distance2(p2)<1e-3) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

/*----------------------------------------------------------------------------*/
cad::GeomManager *
Blocking::geom_model() const {
	return m_geom_model;
}
/*----------------------------------------------------------------------------*/
cad::GeomMeshLinker::eLink Blocking::get_geom_dim(Node &AN) {
	return m_mesh_linker.getGeomDim(AN);
}
/*----------------------------------------------------------------------------*/
cad::GeomMeshLinker::eLink Blocking::get_geom_dim(Edge &AE) {
	return m_mesh_linker.getGeomDim(AE);

}
/*----------------------------------------------------------------------------*/
cad::GeomMeshLinker::eLink Blocking::get_geom_dim(Face &AF) {
	return m_mesh_linker.getGeomDim(AF);
}
/*----------------------------------------------------------------------------*/
int Blocking::get_geom_id(Node &AN) {
	return m_mesh_linker.getGeomId(AN);
}
/*----------------------------------------------------------------------------*/
int Blocking::get_geom_id(Edge &AE) {
	return m_mesh_linker.getGeomId(AE);
}
/*----------------------------------------------------------------------------*/
int Blocking::get_geom_id(Face &AF) {
	return m_mesh_linker.getGeomId(AF);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
void
Blocking::extract_boundary(std::set<TCellID> &ANodeIds, std::set<TCellID> &AEdgeIds, std::set<TCellID> &AFaceIds) {
	ANodeIds.clear();
	AEdgeIds.clear();
	AFaceIds.clear();
	for (auto f_id : m_mesh.faces()) {
		auto f = m_mesh.get<Face>(f_id);
		if (f.nbRegions() == 1) {
			//means we have a boundary face
			AFaceIds.insert(f_id);
			for (auto e_id:  f.getIDs<Edge>())
				AEdgeIds.insert(e_id);
			for (auto n_id:  f.getIDs<Node>())
				ANodeIds.insert(n_id);
		}
	}
}

/*----------------------------------------------------------------------------*/
void
Blocking::move_node(Node AN, math::Point &ALoc) {
	auto [dim, id] =m_mesh_linker.getGeomInfo(AN);
	if (dim == cad::GeomMeshLinker::LinkPoint) {
		// classified onto a point
		cad::GeomPoint *geom_pnt = m_geom_model->getPoint(id);
		AN.setPoint( geom_pnt->point());
	} else if (dim == cad::GeomMeshLinker::LinkCurve) {
		// classified onto a curve
		cad::GeomCurve *geom_curve = m_geom_model->getCurve(id);
		auto p = AN.point();
		geom_curve->project(p);
		AN.setPoint(p);
	} else if (dim == cad::GeomMeshLinker::LinkSurface) {
		// classified onto a surface
		cad::GeomSurface *geom_surf = m_geom_model->getSurface(id);
		auto p = AN.point();
		geom_surf->project(p);
		AN.setPoint(p);
	}
	// otherwise nothing else to do
}

/*----------------------------------------------------------------------------*/
bool
Blocking::is_valid_connected() {
	std::vector<Block> all_blocks,blocks;
	m_mesh.getAll<Region>(all_blocks);
	blocks.push_back(all_blocks[0]);
	std::vector<Block> checked_blocks;
	while (!blocks.empty()) {
		//current block pas dans list checked_blocks
		auto current_block = blocks.back();
		if (std::find(checked_blocks.begin(), checked_blocks.end(), current_block) == checked_blocks.end()) {
			checked_blocks.push_back(current_block);
		}
		blocks.pop_back();
		auto faces = current_block.get<Face>();
		for (const auto& current_face: faces) {
			auto blocks_current_face = current_face.get<Region>();

			//si block current face pas dans check blocs : add dans blocks / sinon ignorer
			for (auto &b_curFace: blocks_current_face) {
				bool in = false;
				if (std::find(checked_blocks.begin(), checked_blocks.end(), b_curFace) == checked_blocks.end()) {
					blocks.push_back(b_curFace);
				}
				/*for(auto b : checked_blocks){

					if(b->info().topo_id == b_curFace->info().topo_id){
						in = true;
					}
				}
				if(!in){
					blocks.push_back(b_curFace);
				}*/
			}
		}
	}

	if (all_blocks.size() == checked_blocks.size())
		return true;
	return false;
}

/*----------------------------------------------------------------------------*/
void Blocking::init_from_bounding_box() {
	m_mesh.clear();

	TCoord min[3] = {MAXFLOAT, MAXFLOAT, MAXFLOAT};
	TCoord max[3] = {-MAXFLOAT, -MAXFLOAT, -MAXFLOAT};
	std::vector<cad::GeomVolume *> vols;
	m_geom_model->getVolumes(vols);
	for (auto v: vols) {
		TCoord v_min[3], v_max[3];
		v->computeBoundingBox(v_min, v_max);
		for (auto i = 0; i < 3; i++)
			if (v_min[i] < min[i]) min[i] = v_min[i];
		for (auto i = 0; i < 3; i++)
			if (v_max[i] > max[i]) max[i] = v_max[i];
	}

	auto n1 = m_mesh.newNode(min[0], min[1], min[2]);
	auto n2 = m_mesh.newNode(min[0], max[1], min[2]);
	auto n3 = m_mesh.newNode(max[0], max[1], min[2]);
	auto n4 = m_mesh.newNode(max[0], min[1], min[2]);
	auto n5 = m_mesh.newNode(min[0], min[1], max[2]);
	auto n6 = m_mesh.newNode(min[0], max[1], max[2]);
	auto n7 = m_mesh.newNode(max[0], max[1], max[2]);
	auto n8 = m_mesh.newNode(max[0], min[1], max[2]);

	m_mesh.newHex(n1, n2, n3, n4, n5, n6, n7, n8);

	MeshDoctor doc(&m_mesh);
	doc.buildFacesAndR2F();
	doc.buildEdgesAndX2E();
	doc.updateUpwardConnectivity();
}

/*----------------------------------------------------------------------------*/
void
Blocking::init_from_mesh(Mesh &AMesh) {
	m_mesh.clear();
	m_mesh=AMesh;
}

/*----------------------------------------------------------------------------*/
void
Blocking::save_vtk_blocking(const std::string &AFileName) {
	IGMeshIOService ios(&m_mesh);

	std::string block_filename = "block_" + AFileName + ".vtk";
	std::string edge_filename = "edge_" + AFileName + ".vtk";
	std::string face_filename = "face_" + AFileName + ".vtk";

	VTKWriter writer_block(&ios);
	writer_block.setCellOptions(N | R);
	writer_block.setDataOptions(N | R);
	writer_block.write(block_filename);

	VTKWriter writer_face(&ios);
	writer_face.setCellOptions(N | F);
	writer_face.setDataOptions(N | F);
	writer_face.write(face_filename);

	VTKWriter writer_edge(&ios);
	writer_edge.setCellOptions(N | E);
	writer_edge.setDataOptions(N | E);
	writer_edge.write(edge_filename);
}

/*----------------------------------------------------------------------------*/
std::vector<std::vector<Blocking::Edge> >
Blocking::get_all_sheet_edge_sets() {
	std::vector<std::vector<Edge> > edges;
	std::vector<Edge> all_edges;
	m_mesh.getAll<Edge>(all_edges);

	std::map<TCellID, bool> edge_done;
	for (auto e: all_edges) {
		edge_done[e.id()] = false;
	}

	bool remain_edge_to_do = true;
	while (remain_edge_to_do) {
		remain_edge_to_do = false;
		// we try and find the first that is not already put into a sheet set
		bool found_edge = false;
		auto edge_index = 0;
		for (auto i = 0; i < all_edges.size() && !found_edge; i++) {
			// we found an edge to treat
			if (edge_done[all_edges[i].id()] == false) {
				found_edge = true;
				edge_index = i;
			}
		}
		if (found_edge) {
			// work to do, we will do another iteration
			remain_edge_to_do = true;
			std::vector<Edge> sh_edges;
			std::map<TCellID, std::pair<TCellID,TCellID> > sh_e2n;
			std::vector<Block> sh_blocks;
			get_sheet_cells(all_edges[edge_index], sh_edges, sh_e2n,sh_blocks);
			// we store the sheet edges
			edges.push_back(sh_edges);
			// now we mark them as treated
			for (auto e: sh_edges) {
				edge_done[e.id()] = true;
			}
		}
	}
	return edges;
}
/*----------------------------------------------------------------------------*/
std::tuple<TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID>
Blocking::get_parallel_edges(Block AB, Edge AE, Node AE_first) {
	auto b_edges = AB.get<Edge>();
	auto b_nodes = AB.get<Node>();
	//we find the local indices of AE end points
	auto e_nodes = AE.get<Node>();
	auto i0 =-1, i1=-1;
	for (auto i=0;i<b_nodes.size();i++) {
		if (b_nodes[i]==e_nodes[0])
			i0 = i;
		else if (b_nodes[i]==e_nodes[1])
			i1 = i;
	}
	assert(i0>=0 && i1>=0);
	//i0 for first node
	if (b_nodes[i0].id()!=AE_first.id()){
		auto tmp = i1;
		i1 = i0;
		i0 = tmp;
	}
	if (b_nodes[i0].id()!=AE_first.id()) {
		throw GMDSException("The given node is not an end point of the edge");
	}
	int lid_e1_n0=-1;
	int lid_e1_n1=-1;
	int lid_e2_n0=-1;
	int lid_e2_n1=-1;
	int lid_e3_n0=-1;
	int lid_e3_n1=-1;
	if (i0==0 && i1==1) {
		lid_e1_n0=3;
		lid_e1_n1=2;
		lid_e2_n0=4;
		lid_e2_n1=5;
		lid_e3_n0=7;
		lid_e3_n1=6;
	}
	else if (i0==1 && i1==0) {
		lid_e1_n0=2;
		lid_e1_n1=3;
		lid_e2_n0=5;
		lid_e2_n1=4;
		lid_e3_n0=6;
		lid_e3_n1=7;
	}
	else if (i0==0 && i1==3) {
		lid_e1_n0=1;
		lid_e1_n1=2;
		lid_e2_n0=4;
		lid_e2_n1=7;
		lid_e3_n0=5;
		lid_e3_n1=6;
	}
	else if (i0==3&& i1==0) {
		lid_e1_n0=2;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=4;
		lid_e3_n0=6;
		lid_e3_n1=5;
	}
	else if (i0==0 && i1==4) {
		lid_e1_n0=1;
		lid_e1_n1=5;
		lid_e2_n0=3;
		lid_e2_n1=7;
		lid_e3_n0=2;
		lid_e3_n1=6;
	}
	else if (i0==4 && i1==0) {
		lid_e1_n0=5;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=3;
		lid_e3_n0=6;
		lid_e3_n1=2;
	}
	else if (i0==1 && i1==2) {
		lid_e1_n0=0;
		lid_e1_n1=3;
		lid_e2_n0=5;
		lid_e2_n1=6;
		lid_e3_n0=4;
		lid_e3_n1=7;
	}
	else if (i0==2 && i1==1) {
		lid_e1_n0=3;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=5;
		lid_e3_n0=7;
		lid_e3_n1=4;
	}
	else if (i0==1 && i1==5) {
		lid_e1_n0=0;
		lid_e1_n1=4;
		lid_e2_n0=2;
		lid_e2_n1=6;
		lid_e3_n0=3;
		lid_e3_n1=7;
	}
	else if (i0==5 && i1==1) {
		lid_e1_n0=4;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=2;
		lid_e3_n0=7;
		lid_e3_n1=3;
	}
	else if (i0==2 && i1==3) {
		lid_e1_n0=1;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=7;
		lid_e3_n0=5;
		lid_e3_n1=4;
	}
	else if (i0==3 && i1==2) {
		lid_e1_n0=0;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=6;
		lid_e3_n0=4;
		lid_e3_n1=5;
	}
	else if (i0==2 && i1==6) {
		lid_e1_n0=1;
		lid_e1_n1=5;
		lid_e2_n0=3;
		lid_e2_n1=7;
		lid_e3_n0=0;
		lid_e3_n1=4;
	}
	else if (i0==6 && i1==2) {
		lid_e1_n0=5;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=3;
		lid_e3_n0=4;
		lid_e3_n1=0;
	}
	else if (i0==3 && i1==7) {
		lid_e1_n0=0;
		lid_e1_n1=4;
		lid_e2_n0=2;
		lid_e2_n1=6;
		lid_e3_n0=1;
		lid_e3_n1=5;
	}
	else if (i0==7 && i1==3) {
		lid_e1_n0=4;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=2;
		lid_e3_n0=5;
		lid_e3_n1=1;
	}
	else if (i0==4 && i1==5) {
		lid_e1_n0=0;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=6;
		lid_e3_n0=3;
		lid_e3_n1=2;
	}
	else if (i0==5 && i1==4) {
		lid_e1_n0=1;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=7;
		lid_e3_n0=2;
		lid_e3_n1=3;
	}
	else if (i0==4 && i1==7) {
		lid_e1_n0=0;
		lid_e1_n1=3;
		lid_e2_n0=5;
		lid_e2_n1=6;
		lid_e3_n0=1;
		lid_e3_n1=2;
	}
	else if (i0==7 && i1==4) {
		lid_e1_n0=3;
		lid_e1_n1=0;
		lid_e2_n0=6;
		lid_e2_n1=5;
		lid_e3_n0=2;
		lid_e3_n1=1;
	}
	else if (i0==5 && i1==6) {
		lid_e1_n0=1;
		lid_e1_n1=2;
		lid_e2_n0=4;
		lid_e2_n1=7;
		lid_e3_n0=0;
		lid_e3_n1=3;
	}
	else if (i0==6 && i1==5) {
		lid_e1_n0=2;
		lid_e1_n1=1;
		lid_e2_n0=7;
		lid_e2_n1=4;
		lid_e3_n0=3;
		lid_e3_n1=0;
	}
	else if (i0==6 && i1==7) {
		lid_e1_n0=2;
		lid_e1_n1=3;
		lid_e2_n0=5;
		lid_e2_n1=4;
		lid_e3_n0=1;
		lid_e3_n1=0;
	}
	else if (i0==7 && i1==6) {
		lid_e1_n0=2;
		lid_e1_n1=3;
		lid_e2_n0=5;
		lid_e2_n1=4;
		lid_e3_n0=1;
		lid_e3_n1=0;
	}
	auto id_e1_n0 = b_nodes[lid_e1_n0].id();
	auto id_e1_n1 = b_nodes[lid_e1_n1].id();
	auto id_e2_n0 = b_nodes[lid_e2_n0].id();
	auto id_e2_n1 = b_nodes[lid_e2_n1].id();
	auto id_e3_n0 = b_nodes[lid_e3_n0].id();
	auto id_e3_n1 = b_nodes[lid_e3_n1].id();
	//now we find the corresponding edges
	TCellID e1,e2,e3, e1_n0, e2_n0, e3_n0, e1_n1, e2_n1, e3_n1;
	for (auto e :b_edges) {
		auto ens = e.getIDs<Node>();
		if (ens[0]==id_e1_n0 && ens[1]==id_e1_n1){
			e1 = e.id();
			e1_n0 = id_e1_n0;
			e1_n1 = id_e1_n1;
		}
		else if (ens[1]==id_e1_n0 && ens[0]==id_e1_n1) {
			e1 = e.id();
			e1_n0 = id_e1_n1;
			e1_n1 = id_e1_n0;

		}
		else if (ens[0]==id_e2_n0 && ens[1]==id_e2_n1) {
			e2 = e.id();
			e2_n0 = id_e2_n0;
			e2_n1 = id_e2_n1;
		}
		else if (ens[1]==id_e2_n0 && ens[0]==id_e2_n1) {
			e2 = e.id();
			e2_n0 = id_e2_n1;
			e2_n1 = id_e2_n0;
		}
		else if (ens[0]==id_e3_n0 && ens[1]==id_e3_n1) {
			e3 = e.id();
			e3_n0 = id_e3_n0;
			e3_n1 = id_e3_n1;
		}
		else if (ens[1]==id_e3_n0 && ens[0]==id_e3_n1) {
			e3 = e.id();
			e3_n0 = id_e3_n1;
			e3_n1 = id_e3_n0;
		}
	}
	return std::make_tuple(e1, e1_n0, e1_n1,
		e2, e2_n0, e2_n1,
		e3, e3_n0, e3_n1);
}

/*----------------------------------------------------------------------------*/
void
Blocking::get_sheet_cells(const Edge AE,
	const Node AE_firstN, std::vector<Edge> &AEdges,
	std::map<TCellID,std::pair<TCellID,TCellID>>& AE2N,
	std::vector<Block>& ABlocks) {
	AEdges.clear();
	AE2N.clear();
	ABlocks.clear();
	// we allocate a mark to know all the edges we go through
	auto edge_mark = m_mesh.newMark<Edge>();

	std::vector<Edge> front;
	front.push_back(AE);
	m_mesh.mark(AE,edge_mark);

	// Now we propagate along topological parallel edges in each adjacent hex cell
	while (!front.empty()) {
		// we pick the last dart of the front
		auto current_edge = front.back();
		front.pop_back();
		//add it in the set of parallel edges
		AEdges.push_back(current_edge);
		// we traverse the adjacent hexes to get parallel edges
		auto adj_hexes = current_edge.get<Region>();
		for (auto h: adj_hexes) {
			ABlocks.push_back(h);
			auto [next_e1, next_e1_n0, next_e1_n1,
				next_e2, next_e2_n0, next_e2_n1,
				next_e3, next_e3_n0, next_e3_n1] =
					get_parallel_edges(h, current_edge, AE_firstN);
			if (!m_mesh.isMarked<Edge>(next_e1,edge_mark)) {
				front.push_back(m_mesh.get<Edge>(next_e1));
			}
			if (!m_mesh.isMarked<Edge>(next_e2,edge_mark)) {
				front.push_back(m_mesh.get<Edge>(next_e2));
			}
			if (!m_mesh.isMarked<Edge>(next_e3,edge_mark)) {
				front.push_back(m_mesh.get<Edge>(next_e3));
			}
		}
	}
	for (auto e:AEdges)
		m_mesh.unmark(e,edge_mark);
	m_mesh.freeMark<Edge>(edge_mark);
}

/*----------------------------------------------------------------------------*/
std::vector<Blocking::Block>
Blocking::get_all_chord_blocks(const Face AF) {
	std::vector<Blocking::Block> bls;

	return bls;
}

/*----------------------------------------------------------------------------*/
std::tuple<Blocking::Edge, double, double>
Blocking::get_cut_info(const int APointId,
                       const std::vector<Blocking::Edge> AEdges) {
	auto point_to_capture = m_geom_model->getPoint(APointId);
	gmds::math::Point p(point_to_capture->X(), point_to_capture->Y(), point_to_capture->Z());
	return get_cut_info(p, AEdges);
}

/*----------------------------------------------------------------------------*/
std::tuple<Blocking::Edge, double, double>
Blocking::get_cut_info(const gmds::math::Point &APoint,
                       const std::vector<Blocking::Edge> AEdges) {
	std::tuple<Blocking::Edge, double, double> cut_param;
	double min_dist = 1000;

	for (auto e: AEdges) {
		auto proj_info = get_projection_info(APoint, e);
		if (proj_info.second < 1 && proj_info.second > 0 && proj_info.first < min_dist) {
			cut_param = std::make_tuple(e, proj_info.second, proj_info.first);
			min_dist = proj_info.first;
		}
	}
	return cut_param;
}

/*----------------------------------------------------------------------------*/
void Blocking::cut_sheet(const TCellID AnEdgeId, const double AParam) {
	cut_sheet(m_mesh.get<Edge>(AnEdgeId), AParam);
}

/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE) {
	cut_sheet(AE, 0.5);
}
/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE, const math::Point &AP) {
	auto ns = AE.get<Node>();
	math::Point p0 = ns[0].point();
	math::Point p1 = ns[1].point();
	math::Segment s01(p0, p1);
	math::Point p = s01.project(AP);
	double param = math::Segment(p0, p).computeLength() / s01.computeLength();
	cut_sheet(AE, param);
}
/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE, const double AParam) {
	assert(AParam > 0 && AParam < 1);
	// Note: the parameterization starts from the first node of AE
	std::vector<Blocking::Edge> sheet_edges;
	std::vector<Blocking::Face> sheet_faces;
	std::vector<Blocking::Block> sheet_blocks;
	get_sheet_cells(AE, sheet_edges,sheet_faces, sheet_blocks);
	auto sheet_mark = m_mesh.newMark<Edge>();
	for (auto e: sheet_edges) {
		m_mesh.mark(e,sheet_mark);
	}

	for (auto b : sheet_blocks) {
		auto b_edges = b.get<Edge>();
		std::vector<Edge> par_edges;
		for (auto e: b_edges) {
			if (m_mesh.isMarked(e, sheet_mark)) {
				par_edges.push_back(e);
			}
		}
		if (par_edges.size() != 4) {
			throw GMDSException("Unsupported blocking cut");
		}
		//We have so 4 parallel edges
	}
	//I unmark all mesh edge, it might be possible to improve that
	m_mesh.unmarkAll<Edge>(sheet_mark);
	m_mesh.freeMark<Edge>(sheet_mark);
}

/*----------------------------------------------------------------------------*/
std::pair<double, double>
Blocking::get_projection_info(const math::Point &AP, Blocking::Edge &AEdge) {

	auto end_points = AEdge.get<Node>();
	math::Point end0 = end_points[0].point();
	math::Point end1 = end_points[1].point();
	math::Vector3d v1 = end1 - end0;
	math::Vector3d v2 = AP - end0;
	double coord = 0.0;
	double distance = 0.0;
	auto a = v1.dot(v2);
	if (a <= 0.0) {
		coord = 0.0;
		distance = AP.distance(end0);
	} else {
		auto b = v1.dot(v1);
		if (a >= b) {
			coord = 1.0;
			distance = AP.distance(end1);
		} else {
			coord = a / b;
			distance = AP.distance(end0 + coord * v1);
		}
	}

	return std::make_pair(distance, coord);
}

/*----------------------------------------------------------------------------*/
std::vector<std::pair<double, double> >
Blocking::get_projection_info(const math::Point &AP, std::vector<Blocking::Edge> &AEdges) {
	std::vector<std::pair<double, double> > dist_coord;
	for (auto e: AEdges) {
		dist_coord.push_back(get_projection_info(AP, e));
	}
	return dist_coord;
}

/*----------------------------------------------------------------------------*/
void
Blocking::smooth(const int ANbIterations) {
	for (auto i = 0; i < ANbIterations; i++) {
		smooth_curves(1);
		smooth_surfaces(1);
		smooth_volumes(1);
	}
}

/*----------------------------------------------------------------------------*/
void
Blocking::smooth_curves(const int ANbIterations) {
	// We traverse all the nodes of the block structure and we keep in mind, those
	// that are on curves
	std::vector<Node> to_smooth;
	for (auto n_id: m_mesh.nodes()){
		auto geom_dim = m_mesh_linker.getGeomDim<Node>(n_id);
		if (geom_dim == cad::GeomMeshLinker::LinkCurve) {
			// node on a curve
			to_smooth.push_back(m_mesh.get<Node>(n_id));
		}
	}
	for (auto i=0; i<ANbIterations; i++) {
		for (auto n: to_smooth) {
			// We get the location of each adjacent node connected through a 1-classified edge
			math::Point p = n.point();
			math::Point deviation(0, 0, 0);
			auto ball_size = 0;
			// We get adjacent nodes by edges
			for (auto e : n.get<Edge>()) {

				auto geom_dim_e = m_mesh_linker.getGeomDim(e);
				if (geom_dim_e == cad::GeomMeshLinker::LinkCurve) {
					// we keep the opposite node
					deviation = deviation + e.getOppositeNode(n).point();
					ball_size++;
				}
							 }
			if (ball_size != 0) {
				// we mode n
				math::Point p_new = 1.0 / (double) ball_size * deviation;
				p = 0.5 * p + 0.5 * p_new;
				move_node(n, p);
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
void
Blocking::smooth_surfaces(const int ANbIterations) {
	// We traverse all the nodes of the block structure and we keep in mind, those
	// that are on surfaces

	std::vector<Node> to_smooth;
	for (auto n_id: m_mesh.nodes()){
		auto geom_dim = m_mesh_linker.getGeomDim<Node>(n_id);
		if (geom_dim == cad::GeomMeshLinker::LinkSurface) {
			// node on a surface
			to_smooth.push_back(m_mesh.get<Node>(n_id));
		}
	}
	for (auto i=0; i<ANbIterations; i++) {
		for (auto n: to_smooth) {
			// We get the location of each adjacent node connected through a 2-classified edge
			math::Point p = n.point();
			math::Point deviation(0, 0, 0);
			auto ball_size = 0;
			// We get adjacent nodes by edges
			for (auto e : n.get<Edge>()) {

				auto geom_dim_e = m_mesh_linker.getGeomDim(e);
				if (geom_dim_e == cad::GeomMeshLinker::LinkSurface) {
					// we keep the opposite node
					deviation = deviation + e.getOppositeNode(n).point();
					ball_size++;
				}
							 }
			if (ball_size != 0) {
				// we mode n
				math::Point p_new = 1.0 / (double) ball_size * deviation;
				p = 0.5 * p + 0.5 * p_new;
				move_node(n, p);
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
void
Blocking::smooth_volumes(const int ANbIterations) {
	// We traverse all the nodes of the block structure and we keep in mind, those
	// that are in volume
	std::vector<Node> to_smooth;
	for (auto n_id: m_mesh.nodes()){
		auto geom_dim = m_mesh_linker.getGeomDim<Node>(n_id);
		if (geom_dim == cad::GeomMeshLinker::LinkVolume) {
			to_smooth.push_back(m_mesh.get<Node>(n_id));
		}
	}
	for (auto i=0; i<ANbIterations; i++) {
		for (auto n: to_smooth) {
			// We get the location of each adjacent node connected through a 2-classified edge
			math::Point p = n.point();
			math::Point deviation(0, 0, 0);
			auto ball_size = 0;
			// We get adjacent nodes by edges
			for (auto e : n.get<Edge>()) {
				// we keep the opposite node
				deviation = deviation + e.getOppositeNode(n).point();
				ball_size++;
			}
			if (ball_size != 0) {
				// we mode n
				math::Point p_new = 1.0 / (double) ball_size * deviation;
				p = 0.5 * p + 0.5 * p_new;
				move_node(n, p);
			}
		}
	}
}

