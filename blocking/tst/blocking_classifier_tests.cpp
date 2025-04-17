#include <gecko/blocking//Blocking.h>
#include <gecko/blocking//BlockingClassifier.h>
#include <gmds/cadfac/FACManager.h>
#include <gmds/ig/MeshDoctor.h>
#include <gmds/igalgo/GridBuilder.h>
#include <gmds/io/IGMeshIOService.h>
#include <gmds/io/VTKReader.h>
#include <gmds/io/VTKWriter.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
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


TEST_CASE("BlockingTestSuite - classify_box", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);


    //peut etre probleme avec destucteur de edges
    auto edges = bl.mesh().edges();
    auto eId = *edges.begin();
    auto e = bl.mesh().get<Edge>(eId);

    auto e_id = bl.get_geom_id(e);
    auto e_dim = bl.get_geom_dim(e);

    REQUIRE(e_dim==cad::GeomMeshLinker::LinkCurve);

    bl.cut_sheet(e);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);

    int classified_nodes = 0;
    int classified_edges = 0;
    for (auto cur_edge_id : bl.mesh().edges()) {
        auto cur_edge = bl.mesh().get<Edge>(cur_edge_id);
        if (bl.get_geom_id(cur_edge) == e_id && bl.get_geom_dim(cur_edge) == e_dim)
            classified_edges++;
    }
    for (auto cur_node_id : bl.mesh().nodes()) {
        auto cur_node = bl.mesh().get<Node>(cur_node_id);
        if (bl.get_geom_id(cur_node) == e_id && bl.get_geom_dim(cur_node) == e_dim)
            classified_nodes++;
    }
    REQUIRE(classified_edges == 2);
    REQUIRE(classified_nodes == 1);

}
/*
TEST_CASE("BlockingTestSuite - split_one_block_twice", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);
    cl.classify();

    auto e = bl.get_all_edges()[0];
    auto e_id = e->info().geom_id;
    auto e_dim = e->info().geom_dim;

    auto e2 = bl.cmap()->attribute<1>(bl.cmap()->beta<1>(e->dart()));
    bl.cut_sheet(e);
    REQUIRE(bl.get_nb_cells<0>() == 12);
    REQUIRE(bl.get_nb_cells<1>() == 20);
    REQUIRE(bl.get_nb_cells<2>() == 11);
    REQUIRE(bl.get_nb_cells<3>() == 2);
    REQUIRE(bl.cmap()->is_valid());

    int classified_nodes = 0;
    int classified_edges = 0;
    for (auto cur_edge : bl.get_all_edges()) {
        if (cur_edge->info().geom_id == e_id && cur_edge->info().geom_dim == e_dim)
            classified_edges++;
    }
    for (auto cur_node : bl.get_all_nodes()) {
        if (cur_node->info().geom_id == e_id && cur_node->info().geom_dim == e_dim)
            classified_nodes++;
    }
    REQUIRE(classified_edges == 2);
    REQUIRE(classified_nodes == 1);

    gmds::math::Point p_cut(2, 5,-5);
    bl.cut_sheet(e2, p_cut);
    for (auto n : bl.get_all_nodes()) {
        auto nz = n->info().point.Z();
        REQUIRE((Approx(nz).margin(1e-4) == 5 || Approx(nz).margin(1e-4) == -5 || Approx(nz).margin(1e-4) == 2));
    }
    REQUIRE(bl.get_nb_cells<0>() == 18);
    REQUIRE(bl.get_nb_cells<1>() == 33);
    REQUIRE(bl.get_nb_cells<2>() == 20);
    REQUIRE(bl.get_nb_cells<3>() == 4);
}

TEST_CASE("BlockingTestSuite - split_until", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);
    cl.classify();

    bool to_cut = true;
    gecko::blocking::Blocking::Edge edge_to_cut;
    while (to_cut) {

        to_cut = false;
        auto all_edges = bl.get_all_edges();
        for (auto cur_edge : all_edges) {
            auto nodes_of_e = bl.get_nodes_of_edge(cur_edge);
            gmds::math::Point p0 = nodes_of_e[0]->info().point;
            gmds::math::Point p1 = nodes_of_e[1]->info().point;
            if (p0.distance(p1) > 2) {
                to_cut = true;
                edge_to_cut = cur_edge;
            }
        }
        if (to_cut) {
            bl.cut_sheet(edge_to_cut, 0.5);
        }
    }
    REQUIRE(bl.get_nb_cells<3>() == 512);
}

TEST_CASE("BlockingTestSuite - init_from_geom_bounding_box", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);

    REQUIRE(bl.get_nb_cells<0>() == 8);
    REQUIRE(bl.get_nb_cells<1>() == 12);
    REQUIRE(bl.get_nb_cells<2>() == 6);
    REQUIRE(bl.get_nb_cells<3>() == 1);

    for (auto a : bl.cmap()->attributes<0>()) {
        gmds::math::Point p = a.info().point;
        REQUIRE(Approx(fabs(p.X())).margin(1e-8) == 5);
        REQUIRE(Approx(fabs(p.Y())).margin(1e-8) == 5);
        REQUIRE(Approx(fabs(p.Z())).margin(1e-8) == 5);
    }
}

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

TEST_CASE("BlockingTestSuite - test_topological_queries", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);

    auto bl_nodes = bl.get_all_nodes();
    auto bl_edges = bl.get_all_edges();
    auto bl_faces = bl.get_all_faces();

    for (auto n : bl_nodes) {
        auto fs = bl.get_faces_of_node(n);
        REQUIRE(fs.size() == 3);

        auto es = bl.get_edges_of_node(n);
        REQUIRE(es.size() == 3);

        auto bs = bl.get_blocks_of_node(n);
        REQUIRE(bs.size() == 1);
    }

    for (auto e : bl_edges) {
        auto ns = bl.get_nodes_of_edge(e);
        REQUIRE(ns.size() == 2);

        auto fs = bl.get_faces_of_edge(e);
        REQUIRE(fs.size() == 2);

        auto bs = bl.get_blocks_of_edge(e);
        REQUIRE(bs.size() == 1);
    }

    for (auto f : bl_faces) {
        auto ns = bl.get_nodes_of_face(f);
        REQUIRE(ns.size() == 4);

        auto es = bl.get_edges_of_face(f);
        REQUIRE(es.size() == 4);

        auto bs = bl.get_blocks_of_face(f);
        REQUIRE(bs.size() == 1);
    }
}


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

    REQUIRE(bl.get_all_nodes().size() == 16);
    REQUIRE(bl.get_all_edges().size() == 28);
    REQUIRE(bl.get_all_faces().size() == 16);
    REQUIRE(bl.get_all_blocks().size() == 3);
}
*/