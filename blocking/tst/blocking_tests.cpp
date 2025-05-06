#include <gecko/blocking/Blocking.h>
#include <gmds/cadfac/FACManager.h>
#include <gmds/ig/MeshDoctor.h>
#include <gmds/igalgo/GridBuilder.h>
#include <gmds/io/IGMeshIOService.h>
#include <gmds/io/VTKReader.h>
#include <gmds/io/VTKWriter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <CGAL/boost/graph/graph_traits_HalfedgeDS_default.h>

#include "../../cblock/inc/gecko/cblock/Blocking.h"
using Catch::Approx;

void
setUp(gmds::cad::FACManager &AGeomManager)
{
	gmds::Mesh m_vol(gmds::MeshModel(gmds::DIM3 | gmds::R | gmds::F | gmds::E | gmds::N |
	                                 gmds::R2N | gmds::R2F | gmds::R2E | gmds::F2N |
	                                 gmds::F2R | gmds::F2E
	                                 | gmds::E2F | gmds::E2N | gmds::N2E));
	std::string dir(TEST_SAMPLES_DIR);
	std::string vtk_file = dir + "/AxisAlign/tet_in_box.vtk";
	gmds::IGMeshIOService ioService(&m_vol);
	gmds::VTKReader vtkReader(&ioService);
	vtkReader.setCellOptions(gmds::N | gmds::R);
	vtkReader.read(vtk_file);
	gmds::MeshDoctor doc(&m_vol);
	doc.buildFacesAndR2F();
	doc.buildEdgesAndX2E();
	doc.updateUpwardConnectivity();

	AGeomManager.initFrom3DMesh(&m_vol);
}

std::tuple<int,int,int,int> get_node_statistics(gecko::blocking::Blocking& ABlocking){
    auto nb_on_vertex=0;
    auto nb_on_curve=0;
    auto nb_on_surface=0;
    auto nb_in_volume=0;
    auto all_nodes = ABlocking.mesh().nodes();
    for(auto n_id:all_nodes){
        auto n = ABlocking.mesh().get<Node>(n_id);
        if (ABlocking.get_geom_dim(n)==cad::GeomMeshLinker::LinkPoint)
            nb_on_vertex++;
        else if (ABlocking.get_geom_dim(n)==cad::GeomMeshLinker::LinkCurve)
            nb_on_curve++;
        else if (ABlocking.get_geom_dim(n)==cad::GeomMeshLinker::LinkSurface)
            nb_on_surface++;
        else if (ABlocking.get_geom_dim(n)==cad::GeomMeshLinker::LinkVolume)
            nb_in_volume++;
    }
    return std::make_tuple(nb_on_vertex,nb_on_curve,nb_on_surface,nb_in_volume);
}

std::tuple<int,int,int> get_edge_statistics(gecko::blocking::Blocking& ABlocking){
    auto nb_on_curve=0;
    auto nb_on_surface=0;
    auto nb_in_volume=0;
    auto all_edges = ABlocking.mesh().edges();
    for(auto e_id:all_edges){
        auto e = ABlocking.mesh().get<Edge>(e_id);
        if (ABlocking.get_geom_dim(e)==cad::GeomMeshLinker::LinkCurve)
            nb_on_curve++;
        else if (ABlocking.get_geom_dim(e)==cad::GeomMeshLinker::LinkSurface)
            nb_on_surface++;
        else if (ABlocking.get_geom_dim(e)==cad::GeomMeshLinker::LinkVolume)
            nb_in_volume++;
    }
    return std::make_tuple(nb_on_curve,nb_on_surface,nb_in_volume);
}

std::tuple<int,int> get_face_statistics(gecko::blocking::Blocking& ABlocking){
    auto nb_on_surface=0;
    auto nb_in_volume=0;
    auto all_faces = ABlocking.mesh().faces();
    for(auto f_id:all_faces){
        auto f = ABlocking.mesh().get<Face>(f_id);
        if (ABlocking.get_geom_dim(f)==cad::GeomMeshLinker::LinkSurface)
            nb_on_surface++;
        else if (ABlocking.get_geom_dim(f)==cad::GeomMeshLinker::LinkVolume)
            nb_in_volume++;
    }
    return std::make_tuple(nb_on_surface,nb_in_volume);
}

void export_vtk(gecko::blocking::Blocking& ABlocking, int AModel, const std::string& AFileName){
    gmds::IGMeshIOService ioService(&ABlocking.mesh());
    gmds::VTKWriter writer(&ioService);
    writer.setCellOptions(AModel);
    writer.setDataOptions(AModel);
    writer.write(AFileName);
}

TEST_CASE("single_block", "[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);


    REQUIRE(bl.mesh().getNbRegions() == 1);
    REQUIRE(bl.mesh().getNbFaces() == 6);
    REQUIRE(bl.mesh().getNbEdges() == 12);
    REQUIRE(bl.mesh().getNbNodes() == 8);

    auto block_center = bl.mesh().get<Region>(0).center();
    for (auto f : bl.mesh().faces()) {
        gmds::math::Point face_center = bl.mesh().get<Face>(f).center();
        REQUIRE(std::abs(block_center.distance(face_center) - 5) < 1e-8);
    }

}

TEST_CASE("remove_single_block", "[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);


    REQUIRE(bl.mesh().getNbRegions() == 1);
    REQUIRE(bl.mesh().getNbFaces() == 6);
    REQUIRE(bl.mesh().getNbEdges() == 12);
    REQUIRE(bl.mesh().getNbNodes() == 8);

    bl.remove_block(bl.mesh().get<Region>(0));


    REQUIRE(bl.mesh().getNbRegions() == 0);
    REQUIRE(bl.mesh().getNbFaces() == 0);
    REQUIRE(bl.mesh().getNbEdges() == 0);
    REQUIRE(bl.mesh().getNbNodes() == 0);
}

TEST_CASE("BlockingTestSuite - split_one_block_once", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    bl.cut_sheet(edges[0]);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);




}

TEST_CASE("BlockingTestSuite - remove_one_of_two_blocks", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    bl.cut_sheet(edges[0]);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);

    bl.remove_block(bl.mesh().get<Region>(1));

    REQUIRE(bl.mesh().getNbRegions() == 1);
    REQUIRE(bl.mesh().getNbFaces() == 6);
    REQUIRE(bl.mesh().getNbEdges() == 12);
    REQUIRE(bl.mesh().getNbNodes() == 8);
}
TEST_CASE("BlockingTestSuite - split_one_block_once_non_uniform", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    bl.cut_sheet(edges[0],edges[0].get<Node>()[0],0.2);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);
    auto nb_10=0, nb_8=0, nb_2=0;
    for (auto ei: bl.mesh().edges()){
        Edge e = bl.mesh().get<Edge>(ei);
        auto le =e.length();
        if(Approx(le).margin(1e-8) == 10)
            nb_10++;
        else if(Approx(le).margin(1e-8) == 8)
            nb_8++;
        else if(Approx(le).margin(1e-8) == 2)
            nb_2++;

    }
    REQUIRE(nb_10==12);
    REQUIRE(nb_8==4);
    REQUIRE(nb_2==4);


    for (auto ri: bl.mesh().regions()){
        Region r = bl.mesh().get<Region>(ri);
        auto vol = r.volume();
        bool is_equal =(Approx(r.volume()).margin(1e-8) == 200 )||
            (Approx(r.volume()).margin(1e-8) == 800 );
        REQUIRE(is_equal);

    }
}

TEST_CASE("BlockingTestSuite - split_one_block_twice", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    auto e = edges[0];
    auto e2 = edges[1];
    bl.cut_sheet(e);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);

    bl.cut_sheet(e2);

    REQUIRE(bl.mesh().getNbRegions()== 4);
    REQUIRE(bl.mesh().getNbNodes() == 18);
    REQUIRE(bl.mesh().getNbEdges() == 33);
    REQUIRE(bl.mesh().getNbFaces() == 20);
}

TEST_CASE("BlockingTestSuite - split_one_block_three", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    auto e = edges[0];
    auto e2 = edges[1];
    auto e3 = edges[8];

    bl.cut_sheet(e);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);

    bl.cut_sheet(e2);

    REQUIRE(bl.mesh().getNbRegions()== 4);
    REQUIRE(bl.mesh().getNbNodes() == 18);
    REQUIRE(bl.mesh().getNbEdges() == 33);
    REQUIRE(bl.mesh().getNbFaces() == 20);

    bl.cut_sheet(e3);
    REQUIRE(bl.mesh().getNbNodes() == 27);
    REQUIRE(bl.mesh().getNbRegions()== 8);
    REQUIRE(bl.mesh().getNbEdges() == 54);
    REQUIRE(bl.mesh().getNbFaces() == 36);

    for (auto e_id:bl.mesh().edges()) {
        Edge ei = bl.mesh().get<Edge>(e_id);
        REQUIRE(ei.getIDs<Face>().size()>1);
        REQUIRE(ei.getIDs<Face>().size()<5);
    }

    edges.clear();
    bl.mesh().getAll<Edge>(edges);
    auto e_cut = edges[0];
    bl.cut_sheet(e_cut);


    for (auto e_id:bl.mesh().edges()) {
        Edge ei = bl.mesh().get<Edge>(e_id);
        REQUIRE(ei.getIDs<Face>().size()>1);
        REQUIRE(ei.getIDs<Face>().size()<5);
    }

    bl.mesh().getAll<Edge>(edges);
    bl.cut_sheet(edges[0]);
}


TEST_CASE("BlockingTestSuite - split_until", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    bool to_cut = true;
    gecko::blocking::Blocking::Edge edge_to_cut;
    while (to_cut) {

        to_cut = false;
        for (auto cur_edge_id : bl.mesh().edges()) {
            Edge cur_edge = bl.mesh().get<Edge>(cur_edge_id);
            auto nodes_of_e = cur_edge.get<Node>();
            gmds::math::Point p0 = nodes_of_e[0].point();
            gmds::math::Point p1 = nodes_of_e[1].point();
            if (p0.distance(p1) > 1) {
                to_cut = true;
                edge_to_cut = cur_edge;
            }
        }
        if (to_cut) {
            bl.cut_sheet(edge_to_cut);
        }
    }
    REQUIRE(bl.mesh().getNbRegions() == 4096);

    for (auto e_id:bl.mesh().edges()) {
        Edge ei = bl.mesh().get<Edge>(e_id);
        REQUIRE(ei.getIDs<Face>().size()>1);
        REQUIRE(ei.getIDs<Face>().size()<5);
    }
    for (auto f_id:bl.mesh().faces()) {
        Face fi = bl.mesh().get<Face>(f_id);
        REQUIRE(fi.getIDs<Edge>().size()==4);
        REQUIRE(fi.getIDs<Region>().size()<3);
    }

    for (auto b_id:bl.mesh().regions() ){
        Region ri = bl.mesh().get<Region>(b_id);
        REQUIRE(ri.getIDs<Face>().size()==6);
    }
}

TEST_CASE("BlockingTestSuite - init_from_geom_bounding_box", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);

    REQUIRE(bl.mesh().getNbNodes() == 8);
    REQUIRE(bl.mesh().getNbEdges() == 12);
    REQUIRE(bl.mesh().getNbFaces() == 6);
    REQUIRE(bl.mesh().getNbRegions() == 1);

    std::vector<gecko::blocking::Blocking::Node> nodes;
    bl.mesh().getAll<Node>(nodes);
    for (auto n : nodes) {
        gmds::math::Point p = n.point();
        REQUIRE(Approx(fabs(p.X())).margin(1e-8) == 5);
        REQUIRE(Approx(fabs(p.Y())).margin(1e-8) == 5);
        REQUIRE(Approx(fabs(p.Z())).margin(1e-8) == 5);
    }
}

//Mesh structure now, useless test ?
/*
TEST_CASE("BlockingTestSuite - single_block_to_mesh", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model);

    gmds::math::Point p000(0, 0, 0);
    gmds::math::Point p010(0, 1, 0);
    gmds::math::Point p110(1, 1, 0);
    gmds::math::Point p100(1, 0, 0);
    gmds::math::Point p001(0, 0, 1);
    gmds::math::Point p011(0, 1, 1);
    gmds::math::Point p111(1, 1, 1);
    gmds::math::Point p101(1, 0, 1);

    bl.create_block(p000, p010, p110, p100, p001, p011, p111, p101);

    gmds::Mesh m(gmds::MeshModel(gmds::DIM3 | gmds::N | gmds::E | gmds::F | gmds::R | gmds::E2N | gmds::F2N | gmds::R2N));
    bl.convert_to_mesh(m);

    REQUIRE(m.getNbNodes() == 8);
    REQUIRE(m.getNbEdges() == 12);
    REQUIRE(m.getNbFaces() == 6);
    REQUIRE(m.getNbRegions() == 1);
}
*/
//Check erreur matching nbReg pour une arrete
TEST_CASE("BlockingTestSuite - test_topological_queries", "[blocking]") {

    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);

    REQUIRE(bl.mesh().getNbNodes() == 8);
    REQUIRE(bl.mesh().getNbEdges() == 12);
    REQUIRE(bl.mesh().getNbFaces() == 6);
    REQUIRE(bl.mesh().getNbRegions() == 1);



    // for (auto n : bl_nodes) {
    //     auto nbFs = bl.mesh().get<Node>(n).nbFaces();
    //     REQUIRE(nbFs == 3);
    //
    //     auto nbEs = bl.mesh().get<Node>(n).nbEdges();
    //     REQUIRE(nbEs == 3);
    //
    //     auto nbBs = bl.mesh().get<Node>(n).nbRegions();
    //     REQUIRE(nbBs == 1);
    // }

    bl.save_vtk_blocking("topological_queries.vtk");
    for (auto &id_e : bl.mesh().edges()) {
        auto e = bl.mesh().get<Edge>(id_e);\

        REQUIRE(e.getIDs<Node>().size() == 2);
        REQUIRE(e.getIDs<Face>().size() == 2);

        //auto bs = bl.get_blocks_of_edge(e);
        int nbBs = 0;

        auto e_faces = e.get<Face>();
        for (auto f : e_faces) {
            if (nbBs < f.getIDs<Region>().size()) {
                nbBs = f.getIDs<Region>().size();
            }
        }
        REQUIRE(nbBs == 1);
    }

    // for (auto f : bl_faces) {
    //     auto ns = bl.get_nodes_of_face(f);
    //     REQUIRE(ns.size() == 4);
    //
    //     auto es = bl.get_edges_of_face(f);
    //     REQUIRE(es.size() == 4);
    //
    //     auto bs = bl.get_blocks_of_face(f);
    //     REQUIRE(bs.size() == 1);
    // }
}

/*
TEST_CASE("BlockingTestSuite - test_equal_operator_blocking", "[BlockingTestSuite}") {
    gmds::cad::FACManager geom_model;
}
*/
TEST_CASE("BlockingTestSuite - test_init_from_ig_mesh", "[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, false);

    gmds::Mesh m(gmds::MeshModel(gmds::DIM3 | gmds::N |  gmds::R | gmds::R2N));
    gmds::Node n0 = m.newNode(gmds::math::Point(0,0,0));
    gmds::Node n1 = m.newNode(gmds::math::Point(1,0,0));
    gmds::Node n2 = m.newNode(gmds::math::Point(1,1,0));
    gmds::Node n3 = m.newNode(gmds::math::Point(0,1,0));

    gmds::Node n4 = m.newNode(gmds::math::Point(0,0,1));
    gmds::Node n5 = m.newNode(gmds::math::Point(1,0,1));
    gmds::Node n6 = m.newNode(gmds::math::Point(1,1,1));
    gmds::Node n7 = m.newNode(gmds::math::Point(0,1,1));

    gmds::Node n8 = m.newNode(gmds::math::Point(0,0,2));
    gmds::Node n9 = m.newNode(gmds::math::Point(1,0,2));
    gmds::Node n10= m.newNode(gmds::math::Point(1,1,2));
    gmds::Node n11= m.newNode(gmds::math::Point(0,1,2));

    gmds::Node n12= m.newNode(gmds::math::Point(0,0,3));
    gmds::Node n13= m.newNode(gmds::math::Point(1,0,3));
    gmds::Node n14= m.newNode(gmds::math::Point(1,1,3));
    gmds::Node n15= m.newNode(gmds::math::Point(0,1,3));

    m.newHex(n0,n1,n2,n3,n4,n5,n6,n7);
    m.newHex(n4,n5,n6,n7,n8,n9,n10,n11);
    m.newHex(n8,n9,n10,n11,n12,n13,n14,n15);

    bl.init_from_mesh(m);

    REQUIRE(bl.mesh().getNbRegions() == 3);
    REQUIRE(bl.mesh().getNbNodes() == 16);
    REQUIRE(bl.mesh().getNbEdges() == 28);
    REQUIRE(bl.mesh().getNbFaces() == 16);
}

TEST_CASE("BlockingTestSuite - test_nb_blocks_from_n","[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);

    std::vector<Node> nodes;
    bl.mesh().getAll<Node>(nodes);
    for (auto n : nodes) {
        auto nBlocks = bl.getBlocks(n);
        REQUIRE(nBlocks.size() == 1);
    }

}

TEST_CASE("BlockingTestSuite - test_sheet_edges","[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);

    std::vector<Edge> sheet_edges;
    std::vector<Edge> edges;
    bl.mesh().getAll<Edge>(edges);
    bl.get_all_sheet_edges(edges[0],sheet_edges);

    REQUIRE(edges.size() == 12);
    REQUIRE(sheet_edges.size() == 4);
}