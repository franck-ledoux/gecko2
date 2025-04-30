/*----------------------------------------------------------------------------*/
#include "gecko/blocking//Blocking.h"

#include <boost/container/detail/pair.hpp>
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
m_mesh(MeshModel(DIM3|R|F|E|N|R2N|F2N|E2N|R2F|F2R|F2E|E2F)),
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
void Blocking::display_info() {
	std::cout<<"----------- Blocking info ----------------"<<std::endl;
	std::cout<<"Nodes "<<std::endl;
	for (auto i:m_mesh.nodes()) {
		auto n = m_mesh.get<Node>(i);
		std::cout<<n.id()<<": "<<n.point()<<std::endl;
	}
	std::cout<<"Edges "<<std::endl;
	for (auto i:m_mesh.edges()) {
		auto e = m_mesh.get<Edge>(i);
		auto ns = e.getIDs<Node>();
		std::cout<<e.id()<<" (n): ";
		for (auto j:ns)
			std::cout<<j<<" ";
		std::cout<<std::endl;
		auto fs = e.getIDs<Face>();
		std::cout<<e.id()<<" (f): ";
		for (auto j:fs)
			std::cout<<j<<" ";
		std::cout<<std::endl;
	}
	std::cout<<"Faces "<<std::endl;
	for (auto i:m_mesh.faces()) {
		auto f= m_mesh.get<Face>(i);
		auto ns = f.getIDs<Node>();
		auto es = f.getIDs<Edge>();
		auto rs = f.getIDs<Region>();
		std::cout<<f.id()<<" (n): ";
		for (auto j:ns)
			std::cout<<j<<" ";
		std::cout<<std::endl;
		std::cout<<f.id()<<" (e): ";
		for (auto j:es)
			std::cout<<j<<" ";
		std::cout<<std::endl;
		std::cout<<f.id()<<" (r): ";
		for (auto j:rs)
			std::cout<<j<<" ";
		std::cout<<std::endl;
	}
	std::cout<<"Blocks "<<std::endl;
	for (auto i:m_mesh.regions()) {
		auto r= m_mesh.get<Region>(i);
		auto ns = r.getIDs<Node>();
		auto fs = r.getIDs<Face>();
		std::cout<<r.id()<<" (n): ";
		for (auto j:ns)
			std::cout<<j<<" ";
		std::cout<<std::endl;
		std::cout<<r.id()<<" (f): ";
		for (auto j:fs)
			std::cout<<j<<" ";
		std::cout<<std::endl;
	}
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
cad::GeomMeshLinker::eLink Blocking::get_geom_dim(Region &AR) {
	return m_mesh_linker.getGeomDim(AR);
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
int Blocking::get_geom_id(Region &AR) {
	return m_mesh_linker.getGeomId(AR);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
void Blocking::set_geom_link(Node &AN, cad::GeomMeshLinker::eLink ADim, int AnId) {
	if (ADim == cad::GeomMeshLinker::eLink::LinkPoint) {
		m_mesh_linker.linkToPoint(AN,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkCurve) {
		m_mesh_linker.linkToCurve(AN,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkSurface) {
		m_mesh_linker.linkToSurface(AN,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkVolume) {
		m_mesh_linker.linkToVolume(AN,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::NoLink) {
		m_mesh_linker.unlinkNode(AN.id());
	}
	else {throw
		GMDSException( "Blocking::set_geom_link: Unknown link type");}
}
/*----------------------------------------------------------------------------*/
void Blocking::set_geom_link(Edge &AE, cad::GeomMeshLinker::eLink ADim, int AnId) {

	if (ADim == cad::GeomMeshLinker::eLink::LinkCurve) {
		m_mesh_linker.linkToCurve(AE,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkSurface) {
		m_mesh_linker.linkToSurface(AE,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkVolume) {
		m_mesh_linker.linkToVolume(AE,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::NoLink) {
		m_mesh_linker.unlinkEdge(AE.id());
	}
	else {throw
		GMDSException( "Blocking::set_geom_link: Unknown link type");}
}

/*----------------------------------------------------------------------------*/
void Blocking::set_geom_link(Face &AF, cad::GeomMeshLinker::eLink ADim, int AnId) {

	if (ADim == cad::GeomMeshLinker::eLink::LinkSurface) {
		m_mesh_linker.linkToSurface(AF,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::LinkVolume) {
		m_mesh_linker.linkToVolume(AF,AnId);
	}
	else if (ADim == cad::GeomMeshLinker::eLink::NoLink) {
		m_mesh_linker.unlinkFace(AF.id());
	}
	else {throw
		GMDSException( "Blocking::set_geom_link: Unknown link type");}
}

/*----------------------------------------------------------------------------*/
void Blocking::set_geom_link(Region &AR, cad::GeomMeshLinker::eLink ADim, int AnId) {

	if (ADim == cad::GeomMeshLinker::eLink::LinkVolume) {
	 	m_mesh_linker.linkToVolume(AR,AnId);
	 }
	else if (ADim == cad::GeomMeshLinker::eLink::NoLink) {
		m_mesh_linker.unlinkRegion(AR.id());
	}
	else {throw
		GMDSException( "Blocking::set_geom_link: Unknown link type");}
}

/*----------------------------------------------------------------------------*/
void
Blocking::extract_boundary(std::set<TCellID> &ANodeIds, std::set<TCellID> &AEdgeIds, std::set<TCellID> &AFaceIds) {
	ANodeIds.clear();
	AEdgeIds.clear();
	AFaceIds.clear();
	for (auto f_id : m_mesh.faces()) {
		auto f = m_mesh.get<Face>(f_id);
		auto f_nbReg = f.nbRegions();
		if (f.getIDs<Region>().size()== 1) {
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
	ALoc = AN.point();
}

/*----------------------------------------------------------------------------*/
Edge Blocking::get_edge(const TCellID ANodeId0, const TCellID ANodeId1) {
	auto edges = m_mesh.edges();
	for (auto e_id: edges) {
		auto e = m_mesh.get<Edge>(e_id);
		auto e_nodes = e.getIDs<Node>();
		auto n0 = e_nodes[0];
		auto n1 = e_nodes[1];
		if (((n0 == ANodeId0) && (n1 == ANodeId1))
			|| ((n0 == ANodeId1) && (n1 == ANodeId0))) {
			return e;
			}
	}
	throw GMDSException("Blocking::get_edge: No edge found");
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
	//node creat on edge -> class of the edge
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
Blocking::init_from_mesh(Mesh& AMesh) {
	m_mesh.clear();
	std::map<TCellID,TCellID> n2n;
	for (auto n_id:AMesh.nodes()) {
		auto from_n = AMesh.get<Node>(n_id);
		auto to_n = m_mesh.newNode(from_n.point());
		n2n[from_n.id()]=to_n.id();
	}
	for (auto r_id:AMesh.regions()) {
		auto from_r = AMesh.get<Region>(r_id);
		auto from_nodes = from_r.getIDs<Node>();
		if (from_nodes.size() != 8) {
			throw GMDSException("The number of nodes must be 8");
		}
		m_mesh.newHex(n2n[from_nodes[0]],n2n[from_nodes[1]],n2n[from_nodes[2]],n2n[from_nodes[3]],
			n2n[from_nodes[4]],n2n[from_nodes[5]],n2n[from_nodes[6]],n2n[from_nodes[7]]);
	}
	MeshDoctor doc(&m_mesh);
	doc.buildFacesAndR2F();
	doc.buildEdgesAndX2E();
	doc.updateUpwardConnectivity();
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
void Blocking::remove_block(Block AB) {
	auto b_faces = AB.get<Face>();
	auto b_nodes = AB.get<Node>();

	m_mesh.deleteRegion(AB);

	for (auto f:b_faces) {
		f.remove(AB);
		if (f.getIDs<Region>().size()==0) {
			//we remove the face, so do we have lonely edges?
			auto f_edges = f.get<Edge>();
			m_mesh.deleteFace(f);
			for (auto e:f_edges) {
				e.remove(f);
				if (e.getIDs<Face>().size()==0) {
					m_mesh.deleteEdge(e);
				}
			}
		}
	}
	//Now we have to check if a node is lonely and must be removed.
	for (auto n_id:m_mesh.nodes()) {
		std::vector<Region> blocks;
		m_mesh.getAll(blocks);
		bool used = false;
		for (auto i=0;i<blocks.size()&& !used;i++) {
			auto b_ns = blocks[i].getIDs<Node>();
			for (auto b_n_id:b_ns) {
				if (n_id==b_n_id)
					used = true;
			}
		}
		if (!used)
			m_mesh.deleteNode(n_id);
	}

}
/*----------------------------------------------------------------------------*/
void Blocking::remove_block(const gmds::TCellID ABlockId) {
	remove_block(m_mesh.get<Region>(ABlockId));

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
			std::map<TCellID, std::pair<TCellID, TCellID>> sh_e2n;
			std::vector<Block> sh_blocks;
			get_sheet_cells(all_edges[edge_index], all_edges[edge_index].get<Node>()[0],
				sh_edges, sh_e2n,sh_blocks);


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
	auto b_faces = AB.get<Face>();
	std::set<Edge> b_edges;
	for (const auto& f:b_faces) {
		auto f_edges = f.get<Edge>();
		for (const auto& e: f_edges) {
			b_edges.insert(e);
		}
	}
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
	if(i0<0 || i1<0) {
		throw GMDSException("Impposible to find this edge in this block");
	}
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
		lid_e1_n0=3;
		lid_e1_n1=2;
		lid_e2_n0=4;
		lid_e2_n1=5;
		lid_e3_n0=0;
		lid_e3_n1=1;
	}
	else {
		throw GMDSException("Enumeration error!!!");
	}
	auto id_e1_n0 = b_nodes[lid_e1_n0].id();
	auto id_e1_n1 = b_nodes[lid_e1_n1].id();
	auto id_e2_n0 = b_nodes[lid_e2_n0].id();
	auto id_e2_n1 = b_nodes[lid_e2_n1].id();
	auto id_e3_n0 = b_nodes[lid_e3_n0].id();
	auto id_e3_n1 = b_nodes[lid_e3_n1].id();
	//now we find the corresponding edges
	TCellID e1,e2,e3;
	for (auto e :b_edges) {
		auto ens = e.getIDs<Node>();
		if ((ens[0]==id_e1_n0 && ens[1]==id_e1_n1)||
			(ens[1]==id_e1_n0 && ens[0]==id_e1_n1)){
			e1 = e.id();
		}
		else if ((ens[0]==id_e2_n0 && ens[1]==id_e2_n1)||
			(ens[1]==id_e2_n0 && ens[0]==id_e2_n1)){
			e2 = e.id();
		}
		else if ((ens[0]==id_e3_n0 && ens[1]==id_e3_n1) ||
			(ens[1]==id_e3_n0 && ens[0]==id_e3_n1)) {
			e3 = e.id();
		}
	}
	return std::make_tuple(e1, id_e1_n0, id_e1_n1,
		e2, id_e2_n0, id_e2_n1,
		e3, id_e3_n0, id_e3_n1);
}

/*----------------------------------------------------------------------------*/
void
Blocking::get_sheet_cells(const Edge AE,
	const Node AE_firstN,
	std::vector<Edge> &AEdges,
	std::map<TCellID,std::pair<TCellID,TCellID>>& AE2N,
	std::vector<Block>& ABlocks) {
	AEdges.clear();
	AE2N.clear();
	ABlocks.clear();
	// we allocate a mark to know all the edges we go through
	auto edge_mark = m_mesh.newMark<Edge>();
	auto block_mark = m_mesh.newMark<Region>();

	std::vector<std::pair<Edge,Node> > front;
	front.push_back(std::make_pair(AE,AE_firstN));
	AEdges.push_back(AE);
	auto AE_sndN =	AE.getOppositeNode(AE_firstN);
	AE2N[AE.id()]={AE_firstN.id(), AE_sndN.id()};
	m_mesh.mark(AE,edge_mark);

	// Now we propagate along topological parallel edges in each adjacent hex cell
	while (!front.empty()) {
		// we pick the last dart of the front
		auto current_edge_node = front.back();
		front.pop_back();
		// we traverse the adjacent hexes to get parallel edges
		auto adj_faces = current_edge_node.first.get<Face>();
		std::set<Region> adj_hexes;
		for (auto f:adj_faces) {
			std::vector<Region> f_regions = f.get<Region>();
			for (auto r:f_regions) {
				if (!m_mesh.isMarked(r,block_mark)) {
					adj_hexes.insert(r);
				}
			}
		}
		for (auto h: adj_hexes) {
			ABlocks.push_back(h);
			m_mesh.mark(h,block_mark);
			auto [next_e1, next_e1_n0, next_e1_n1,
				next_e2, next_e2_n0, next_e2_n1,
				next_e3, next_e3_n0, next_e3_n1] =
					get_parallel_edges(h, current_edge_node.first,
						current_edge_node.second);
			if (!m_mesh.isMarked<Edge>(next_e1,edge_mark)) {
				front.push_back(std::make_pair(m_mesh.get<Edge>(next_e1),
					m_mesh.get<Node>(next_e1_n0)));
				AEdges.push_back(m_mesh.get<Edge>(next_e1));
				AE2N[next_e1]={next_e1_n0, next_e1_n1};
				m_mesh.mark<Edge>(next_e1,edge_mark);


			}
			if (!m_mesh.isMarked<Edge>(next_e2,edge_mark)) {
				front.push_back(std::make_pair(m_mesh.get<Edge>(next_e2),
					m_mesh.get<Node>(next_e2_n0)));
				AEdges.push_back(m_mesh.get<Edge>(next_e2));
				AE2N[next_e2]={next_e2_n0, next_e2_n1};
				m_mesh.mark<Edge>(next_e2,edge_mark);
			}
			if (!m_mesh.isMarked<Edge>(next_e3,edge_mark)) {
				front.push_back(std::make_pair(m_mesh.get<Edge>(next_e3),
					m_mesh.get<Node>(next_e3_n0)));
				AEdges.push_back(m_mesh.get<Edge>(next_e3));
				AE2N[next_e3]={next_e3_n0, next_e3_n1};
				m_mesh.mark<Edge>(next_e3,edge_mark);
			}
		}
	}
	for (auto e:AEdges)
		m_mesh.unmark(e,edge_mark);
	m_mesh.freeMark<Edge>(edge_mark);
	for (auto b:ABlocks)
		m_mesh.unmark(b,block_mark);
	m_mesh.freeMark<Region>(block_mark);
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
void Blocking::cut_sheet(const TCellID AnEdgeId, const TCellID ANodeId, const double AParam) {
	cut_sheet(m_mesh.get<Edge>(AnEdgeId),
		m_mesh.get<Node>(ANodeId),
		AParam);
}

/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE) {
	cut_sheet(AE, AE.get<Node>()[0], 0.5);
}
/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE, const Node AN, const math::Point &AP) {
	auto ns = AE.get<Node>();
	math::Point p0, p1;
	if (ns[0]==AN) {
		p0 = ns[0].point();
		p1 = ns[1].point();
	}
	else {
		p0 = ns[1].point();
		p1 = ns[0].point();
	}
	math::Segment s01(p0, p1);
	math::Point p = s01.project(AP);
	double param = math::Segment(p0, p).computeLength() / s01.computeLength();
	cut_sheet(AE, AN, param);
}
/*----------------------------------------------------------------------------*/
void
Blocking::cut_sheet(const Edge AE, const Node AN, const double AParam) {
	assert(AParam > 0 && AParam < 1);
	// Note: the parameterization starts from the first node of AE
	std::vector<Blocking::Edge> sheet_edges;
	std::map<TCellID, std::pair<TCellID, TCellID>> sheet_e2N;
	std::vector<Blocking::Block> sheet_blocks;
	get_sheet_cells(AE, AN, sheet_edges,sheet_e2N, sheet_blocks);
	//for each sheet edge, we store the link from the first to the second node
	std::map<TCellID,TCellID> sheet_n2n;
	//link each first node to the edge
	std::map<TCellID,TCellID> sheet_n2e;
	for (auto e2nn: sheet_e2N) {
		sheet_n2n[e2nn.second.first]= e2nn.second.second;
		sheet_n2e[e2nn.second.first]= e2nn.first;
	}

	auto mark_sheet_edges = m_mesh.newMark<Edge>();
	for (auto e: sheet_edges) {
		m_mesh.mark(e,mark_sheet_edges);
	}

	for (auto b : sheet_blocks) {
		auto b_faces = b.get<Face>();
		std::set<Edge> b_edges;
		for (const auto& fb:b_faces) {
			for (const auto& e_fb:fb.get<Edge>())
				b_edges.insert(e_fb);
		}
		std::vector<Edge> par_edges;
		for (auto e: b_edges) {
			if (m_mesh.isMarked(e, mark_sheet_edges)) {
				par_edges.push_back(e);
			}
		}
		if (par_edges.size() != 4) {
			//clean the boolean marks
			m_mesh.unmarkAll<Edge>(mark_sheet_edges);
			m_mesh.freeMark<Edge>(mark_sheet_edges);
			throw GMDSException("Unsupported blocking cut");
		}
		//We have so 4 parallel edges
	}
	//we retrieve all the nodes that are at the beginning of the edges to cut (and
	//so on the same side of the cut)
	auto mark_first_node = m_mesh.newMark<Node>();
	for (auto e2n_info: sheet_e2N) {
		m_mesh.mark<Node>(e2n_info.second.first, mark_first_node);
	}
	//We retrieve the set of faces that defines the surface where we are going
	//to insert the new layer of hexes. For each face, we keep in mind its adjacent region
	//that belongs to the sheet to be cut
	std::map<Blocking::Face, Blocking::Block> sheet_faces;

	for (auto b: sheet_blocks) {
		auto b_fs = b.get<Face>();
		for (auto f: b_fs) {
			auto f_node_ids = f.getIDs<Node>();
			auto is_sheet_face = true;
			for (auto n_id: f_node_ids) {
				if (!m_mesh.isMarked<Node>(n_id, mark_first_node)) {
					is_sheet_face = false;
				}
			}
			if (is_sheet_face) {
				sheet_faces[f]=b;
			}
		}
	}
	//now we get the opposite face
	std::map<TCellID, TCellID> sheet_opp_faces;
	std::set<TCellID> sheet_inside_faces;
	std::map<std::pair<TCellID,TCellID>,TCellID> map_nodes_to_inside_face;

	for (const auto& fr: sheet_faces) {
		auto f = fr.first;
		auto r = fr.second;
		std::vector<TCellID> f_node_ids = f.getIDs<Node>();
		TCellID opp_f_id = NullID;
		for (const auto& fi : r.get<Face>()) {
			if (fi!=f) {
				std::vector<TCellID> fi_node_ids = fi.getIDs<Node>();
				// fi is opposite to f if all its node are different of f nodes
				auto all_different = true;
				for (auto n_id: fi_node_ids) {
					for (auto n2_id: f_node_ids) {
						if (n2_id==n_id) {
							all_different = false;
						}
					}
				}
				if (all_different) {
					sheet_opp_faces[f.id()]=fi.id();
				}
				else {
					sheet_inside_faces.insert(fi.id());
					for (auto k=0;k<4;k++) {
						auto id_k1 = fi_node_ids[k];
						auto id_k2 = fi_node_ids[(k+1)%4];
						if ((m_mesh.isMarked<Node>(id_k1, mark_first_node)) &&
							(m_mesh.isMarked<Node>(id_k2, mark_first_node))) {
							auto id_kk = (id_k1<id_k2)?std::make_pair(id_k1,id_k2):std::make_pair(id_k2,id_k1);
							map_nodes_to_inside_face[id_kk]=fi.id();
							}
					}

				}
			}
		}
	}
	// We get the origin and opposite edges
	std::map<std::pair<TCellID,TCellID>,TCellID> map_nodes_to_origin_edge;
	std::map<std::pair<TCellID,TCellID>,TCellID> map_nodes_to_opposite_edge;
	for (const auto& fr: sheet_faces) {
		auto f = fr.first;
		auto f_edges = f.get<Edge>();
		for (const auto &ei: f_edges) {
			auto ei_node_ids = ei.getIDs<Node>();
			auto n_id0 = (ei_node_ids[0]<ei_node_ids[1])?ei_node_ids[0]:ei_node_ids[1];
			auto n_id1 = (ei_node_ids[0]<ei_node_ids[1])?ei_node_ids[1]:ei_node_ids[0];
			map_nodes_to_origin_edge[std::make_pair(n_id0,n_id1)]=ei.id();
		}
		auto f_opp = m_mesh.get<Face>(sheet_opp_faces[f.id()]);
		auto f_opp_edges = f_opp.get<Edge>();
		for (const auto &ei: f_opp_edges) {

			auto ei_node_ids = ei.getIDs<Node>();
			auto n_id0 = (ei_node_ids[0]<ei_node_ids[1])?ei_node_ids[0]:ei_node_ids[1];
			auto n_id1 = (ei_node_ids[0]<ei_node_ids[1])?ei_node_ids[1]:ei_node_ids[0];
			map_nodes_to_opposite_edge[std::make_pair(n_id0,n_id1)]=ei.id();
		}

	}

	//we build the new cells
	std::map<TCellID,TCellID> map_node_to_new_node;
	std::map<TCellID,TCellID> map_node_to_new_first_edge;
	std::map<TCellID,TCellID> map_node_to_new_second_edge;
	std::map<std::pair<TCellID,TCellID>,TCellID> map_edge_to_new_edge;
	std::map<std::pair<TCellID,TCellID>,TCellID> map_edge_to_new_first_face;
	std::map<std::pair<TCellID,TCellID>,TCellID> map_edge_to_new_second_face;
	auto mark_done_node = m_mesh.newMark<Node>();
	auto mark_done_edge = m_mesh.newMark<Edge>();
	for (const auto& fr: sheet_faces) {
		auto f = fr.first;
		auto r = fr.second;
		auto f_opp = m_mesh.get<Face>(sheet_opp_faces[f.id()]);
		auto f_node_ids = f.getIDs<Node>();
		auto f_opp_node_ids = f_opp.getIDs<Node>();
		auto f_edges = f.get<Edge>();
		auto f_opp_edges = f_opp.get<Edge>();
		std::vector<Blocking::Node> new_nodes(4);
		std::vector<Blocking::Edge> new_first_edges(4);
		std::vector<Blocking::Edge> new_second_edges(4);
		std::vector<Blocking::Face> new_first_faces(4);
		std::vector<Blocking::Face> new_second_faces(4);
		std::vector<Blocking::Edge> new_edges(4);
		// We create new points and new edges if they are not already created
		for (auto i=0;i<4;i++) {
			if (m_mesh.isMarked<Node>(f_node_ids[i], mark_done_node)) {
				new_nodes[i] = m_mesh.get<Node>(map_node_to_new_node[f_node_ids[i]]);
				new_first_edges[i] = m_mesh.get<Edge>(map_node_to_new_first_edge[f_node_ids[i]]);
				new_second_edges[i] = m_mesh.get<Edge>(map_node_to_new_first_edge[f_node_ids[i]]);
			}
			else {
				auto p1 = m_mesh.get<Node>(f_node_ids[i]).point();
				auto p2 = m_mesh.get<Node>(sheet_n2n[f_node_ids[i]]).point();
				new_nodes[i] = m_mesh.newNode((1-AParam)*p1+AParam*p2);
				auto old_e_id = sheet_n2e[f_node_ids[i]];
				auto old_e = m_mesh.get<Edge>(old_e_id);
				set_geom_link(new_nodes[i], get_geom_dim(old_e), get_geom_id(old_e));

				new_first_edges[i] = m_mesh.newEdge(m_mesh.get<Node>(f_node_ids[i]), new_nodes[i]);
				set_geom_link(new_first_edges[i], get_geom_dim(old_e), get_geom_id(old_e));
				new_second_edges[i] = m_mesh.newEdge(new_nodes[i], m_mesh.get<Node>(sheet_n2n[f_node_ids[i]]));
				set_geom_link(new_second_edges[i], get_geom_dim(old_e), get_geom_id(old_e));
				//update maps
				map_node_to_new_node[f_node_ids[i]] = new_nodes[i].id();
				map_node_to_new_first_edge[f_node_ids[i]] = new_first_edges[i].id();
				map_node_to_new_second_edge[f_node_ids[i]] = new_second_edges[i].id();

			}
		}
		// We create new faces if mandatory
		for (auto i=0;i<4;i++) {
			auto id_i = f_node_ids[i];
			auto id_j = f_node_ids[(i+1)%4];
			auto id_ij  = (id_i<id_j)?std::make_pair(id_i,id_j):
							std::make_pair(id_j,id_i);
			auto old_face_id = map_nodes_to_inside_face[id_ij];
			auto old_face = m_mesh.get<Face>(old_face_id);
			if (m_mesh.isMarked<Edge>(map_nodes_to_origin_edge[id_ij], mark_done_edge)) {
				new_edges[i] = m_mesh.get<Edge>(map_edge_to_new_edge[id_ij]);
				new_first_faces[i] = m_mesh.get<Face>(map_edge_to_new_first_face[id_ij]);
				new_second_faces[i] = m_mesh.get<Face>(map_edge_to_new_second_face[id_ij]);
			}
			else {

				new_edges[i] = m_mesh.newEdge(map_node_to_new_node[id_ij.first],
					map_node_to_new_node[id_ij.second]);
				set_geom_link(new_edges[i], get_geom_dim(old_face), get_geom_id(old_face));

				new_first_faces[i] = m_mesh.newQuad(id_ij.first, id_ij.second,
					map_node_to_new_node[id_ij.second],map_node_to_new_node[id_ij.first]);
				set_geom_link(new_first_faces[i], get_geom_dim(old_face), get_geom_id(old_face));

				new_second_faces[i] = m_mesh.newQuad(map_node_to_new_node[id_ij.first],
					map_node_to_new_node[id_ij.second],
					sheet_n2n[id_ij.second],
					sheet_n2n[id_ij.first]);
				set_geom_link(new_second_faces[i], get_geom_dim(old_face), get_geom_id(old_face));
				//update maps
				map_edge_to_new_edge[id_ij] = new_edges[i].id();
				map_edge_to_new_first_face[id_ij] = new_first_faces[i].id();
				map_edge_to_new_second_face[id_ij] = new_second_faces[i].id();
			}
		}
		//TODO class reg
		// now we create the inner face and the two new blocks
		auto new_inner_face = m_mesh.newQuad(new_nodes[0], new_nodes[1], new_nodes[2], new_nodes[3]);

		set_geom_link(new_inner_face, get_geom_dim(r), get_geom_id(r));

		auto first_block = m_mesh.newHex(
			f_node_ids[0], f_node_ids[1],
			f_node_ids[2], f_node_ids[3],
			map_node_to_new_node[f_node_ids[0]], map_node_to_new_node[f_node_ids[1]],
			map_node_to_new_node[f_node_ids[2]], map_node_to_new_node[f_node_ids[3]]);
		set_geom_link(first_block, get_geom_dim(r), get_geom_id(r));

		auto snd_block = m_mesh.newHex(
			map_node_to_new_node[f_node_ids[0]], map_node_to_new_node[f_node_ids[1]],
			map_node_to_new_node[f_node_ids[2]], map_node_to_new_node[f_node_ids[3]],
			sheet_n2n[f_node_ids[0]], sheet_n2n[f_node_ids[1]],
			sheet_n2n[f_node_ids[2]], sheet_n2n[f_node_ids[3]]);

		set_geom_link(snd_block, get_geom_dim(r), get_geom_id(r));
		//Remove cells and update connectivities

		//F2R and R2F
		f.replace<Region>(r.id(), first_block.id());

		m_mesh.get<Face>(sheet_opp_faces[f.id()]).replace<Region>(r.id(), snd_block.id());
		if (!new_inner_face.has(first_block))
			new_inner_face.add(first_block);
		if (!new_inner_face.has(snd_block))
			new_inner_face.add(snd_block);
		if (!first_block.has(f))
			first_block.add<Face>(f.id());
		if (!first_block.has(new_inner_face))
			first_block.add<Face>(new_inner_face.id());
		if (!snd_block.has<Face>(sheet_opp_faces[f.id()]))
			snd_block.add<Face>(sheet_opp_faces[f.id()]);
		if (!snd_block.has<Face>(new_inner_face))
			snd_block.add<Face>(new_inner_face.id());
		for (auto &f_lateral: new_first_faces) {
				if (!f_lateral.has(first_block))
					f_lateral.add(first_block);
				if (!first_block.has(f_lateral))
					first_block.add(f_lateral);

		}
		for (auto &f_lateral: new_second_faces) {

				if (!f_lateral.has(snd_block))
					f_lateral.add(snd_block);

				if (!snd_block.has(f_lateral))
					snd_block.add(f_lateral);

		}

		// F2E| E2F
		for (auto i = 0; i < 4; i++) {
			auto id_i = f_node_ids[i];
			auto id_j = f_node_ids[(i + 1) % 4];
			auto opp_i = sheet_n2n[id_i];
			auto opp_j = sheet_n2n[id_j];
			auto id_ij = (id_i < id_j) ? std::make_pair(id_i, id_j) : std::make_pair(id_j, id_i);
			auto opp_ij = (opp_i < opp_j) ? std::make_pair(opp_i, opp_j) : std::make_pair(opp_j, opp_i);

			Edge origin_edge = m_mesh.get<Edge>(map_nodes_to_origin_edge[id_ij]);
			Edge opp_edge = m_mesh.get<Edge>(map_nodes_to_opposite_edge[opp_ij]);
			auto inner_edge = m_mesh.get<Edge>(map_edge_to_new_edge[id_ij]);
			new_inner_face.add<Edge>(inner_edge);
			inner_edge.add(new_inner_face);

			auto first_face = m_mesh.get<Face>(map_edge_to_new_first_face[id_ij]);
			auto snd_face = m_mesh.get<Face>(map_edge_to_new_second_face[id_ij]);

			if (!first_face.has(inner_edge))
				first_face.add(inner_edge);

			if (!snd_face.has(inner_edge))
				snd_face.add(inner_edge);

			if (!inner_edge.has(first_face))
				inner_edge.add(first_face);

			if (!inner_edge.has(snd_face))
				inner_edge.add(snd_face);

			//boundary edges
			auto e_i_first = m_mesh.get<Edge>(map_node_to_new_first_edge[id_ij.first]);
			auto e_i_snd = m_mesh.get<Edge>(map_node_to_new_second_edge[id_ij.first]);
			auto e_j_first = m_mesh.get<Edge>(map_node_to_new_first_edge[id_ij.second]);
			auto e_j_snd = m_mesh.get<Edge>(map_node_to_new_second_edge[id_ij.second]);

			if (!e_i_first.has(first_face))
				e_i_first.add(first_face);

			if (!first_face.has(e_i_first))
				first_face.add(e_i_first);

			if (!e_j_first.has(first_face))
				e_j_first.add(first_face);

			if (!first_face.has(e_j_first))
				first_face.add(e_j_first);

			if (!e_i_snd.has(snd_face))
				e_i_snd.add(snd_face);

			if (!snd_face.has(e_i_snd))
				snd_face.add(e_i_snd);

			if (!e_j_snd.has(snd_face))
				e_j_snd.add(snd_face);

			if (!snd_face.has(e_j_snd))
				snd_face.add(e_j_snd);

			// the original and opposite edges

			if (!origin_edge.has(first_face))
				origin_edge.add(first_face);

			if (!first_face.has(origin_edge))
				first_face.add(origin_edge);

			if (!opp_edge.has(snd_face))
				opp_edge.add(snd_face);

			if (!snd_face.has(opp_edge))
				snd_face.add(opp_edge);
		}
		//we mark the node and edge for the next blocks
		for (auto nid: f_node_ids)
			m_mesh.mark<Node>(nid, mark_done_node);
		for (auto e: f_edges)
			m_mesh.mark(e, mark_done_edge);
	}

	//delete cells
	//R
	for (const auto &info: sheet_faces) {
		auto current_r = info.second;
		for (auto f: current_r.get<Face>()) {
			f.remove(current_r);
			current_r.remove(f);
		}
		m_mesh.deleteRegion(current_r);
	}
	//F
	for (const auto &f_id: sheet_inside_faces) {
		auto f = m_mesh.get<Face>(f_id);
		for (auto r: f.get<Region>()) {
			r.remove(f);
		}
		for (auto e: f.get<Edge>()) {
			e.remove(f);
			f.remove(e);
		}
		m_mesh.deleteFace(f);
	}
	//E
	for (auto &e: sheet_edges) {
		for (auto f: e.get<Face>()) {
			f.remove(e);
		}
		m_mesh.deleteEdge(e);
	}

	//================================================================
	// mark cleaning now
	//================================================================
/*	for (auto e2n_info: sheet_e2N) {
		m_mesh.unmark<Node>(e2n_info.second.first, mark_first_node);
		m_mesh.unmark<Node>(e2n_info.second.first, mark_done_node);
	}*/
	m_mesh.unmarkAll<Node>(mark_first_node);
	m_mesh.unmarkAll<Node>(mark_done_node);
	m_mesh.freeMark<Node>(mark_first_node);
	m_mesh.freeMark<Node>(mark_done_node);

	//I unmark all mesh edge, it might be possible to improve that
	m_mesh.unmarkAll<Edge>(mark_done_edge);
	m_mesh.freeMark<Edge>(mark_done_edge);
	m_mesh.unmarkAll<Edge>(mark_sheet_edges);
	m_mesh.freeMark<Edge>(mark_sheet_edges);
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

