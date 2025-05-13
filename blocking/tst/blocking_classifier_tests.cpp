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
#include <iostream>
using Catch::Approx;

void
setUp_class(gmds::cad::FACManager &AGeomManager)
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

std::tuple<int,int,int,int> get_node_statistics_class(gecko::blocking::Blocking& ABlocking){
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

std::tuple<int,int,int> get_edge_statistics_class(gecko::blocking::Blocking& ABlocking){
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

std::tuple<int,int> get_face_statistics_class(gecko::blocking::Blocking& ABlocking){
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

void export_vtk_class(gecko::blocking::Blocking& ABlocking, int AModel, const std::string& AFileName){
    gmds::IGMeshIOService ioService(&ABlocking.mesh());
    gmds::VTKWriter writer(&ioService);
    writer.setCellOptions(AModel);
    writer.setDataOptions(AModel);
    writer.write(AFileName);
}


TEST_CASE("BlockingTestSuite - classify_box", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    REQUIRE(m_boundary_node_ids.size() == 8);
    REQUIRE(m_boundary_edge_ids.size() == 12);
    REQUIRE(m_boundary_face_ids.size() == 6);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
}

TEST_CASE("BlockingTestSuite - split_and_capt_with_reset", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    REQUIRE(m_boundary_node_ids.size() == 8);
    REQUIRE(m_boundary_edge_ids.size() == 12);
    REQUIRE(m_boundary_face_ids.size() == 6);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);

    std::vector<Edge> bl_edges;
    bl.mesh().getAll<Edge>(bl_edges);

    auto e = bl_edges[0];

    auto e_id = bl.get_geom_id(e);
    auto e_dim = bl.get_geom_dim(e);

    std::vector<Node> bl_nodes;
    bl.mesh().getAll<Node>(bl_nodes);
    auto n =bl_nodes[0];

    auto n_id = bl.get_geom_id(n);
    auto n_dim = bl.get_geom_dim(n);

    REQUIRE(n_dim==cad::GeomMeshLinker::LinkPoint);
    REQUIRE(e_dim==cad::GeomMeshLinker::LinkCurve);

    bl.cut_sheet(e);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);


    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);
    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
}

TEST_CASE("BlockingTestSuite - split_and_capt_without_reset", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    REQUIRE(m_boundary_node_ids.size() == 8);
    REQUIRE(m_boundary_edge_ids.size() == 12);
    REQUIRE(m_boundary_face_ids.size() == 6);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);

    std::vector<Edge> bl_edges;
    bl.mesh().getAll<Edge>(bl_edges);

    auto e = bl_edges[0];

    auto e_id = bl.get_geom_id(e);
    auto e_dim = bl.get_geom_dim(e);

    std::vector<Node> bl_nodes;
    bl.mesh().getAll<Node>(bl_nodes);
    auto n =bl_nodes[0];

    auto n_id = bl.get_geom_id(n);
    auto n_dim = bl.get_geom_dim(n);

    REQUIRE(n_dim==cad::GeomMeshLinker::LinkPoint);
    REQUIRE(e_dim==cad::GeomMeshLinker::LinkCurve);

    bl.cut_sheet(e);
    REQUIRE(bl.mesh().getNbNodes() == 12);
    REQUIRE(bl.mesh().getNbEdges() == 20);
    REQUIRE(bl.mesh().getNbFaces() == 11);
    REQUIRE(bl.mesh().getNbRegions()== 2);

    errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
}
TEST_CASE("BlockingTestSuite - split_until_and_capt_without_reset", "[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::vector<gecko::blocking::Blocking::Edge> edges;
    bl.mesh().getAll<Edge>(edges);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    REQUIRE(m_boundary_node_ids.size() == 8);
    REQUIRE(m_boundary_edge_ids.size() == 12);
    REQUIRE(m_boundary_face_ids.size() == 6);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);


    bool to_cut = true;
    gecko::blocking::Blocking::Edge edge_to_cut;
    while (to_cut) {

        to_cut = false;
        for (auto cur_edge_id : bl.mesh().edges()) {
            Edge cur_edge = bl.mesh().get<Edge>(cur_edge_id);
            auto nodes_of_e = cur_edge.get<Node>();
            gmds::math::Point p0 = nodes_of_e[0].point();
            gmds::math::Point p1 = nodes_of_e[1].point();
            if (p0.distance(p1) > 3) {
                to_cut = true;
                edge_to_cut = cur_edge;
            }
        }
        if (to_cut) {
            bl.cut_sheet(edge_to_cut);
        }
    }
    REQUIRE(bl.mesh().getNbRegions() == 64);

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

    errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
}


TEST_CASE("BlockingTestSuite - split_three_and_finale_capt","[blocking]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model,true);

    bool to_cut = true;
    gecko::blocking::Blocking::Edge edge_to_cut;
    while (to_cut) {

        to_cut = false;
        for (auto cur_edge_id : bl.mesh().edges()) {
            Edge cur_edge = bl.mesh().get<Edge>(cur_edge_id);
            auto nodes_of_e = cur_edge.get<Node>();
            gmds::math::Point p0 = nodes_of_e[0].point();
            gmds::math::Point p1 = nodes_of_e[1].point();
            if (p0.distance(p1) > 5) {
                to_cut = true;
                edge_to_cut = cur_edge;
            }
        }
        if (to_cut) {
            bl.cut_sheet(edge_to_cut);
        }
    }
    REQUIRE(bl.mesh().getNbRegions() == 8);

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
    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    REQUIRE(m_boundary_node_ids.size() == 26);
    REQUIRE(m_boundary_edge_ids.size() == 48);
    REQUIRE(m_boundary_face_ids.size() == 24);

    gecko::blocking::BlockingClassifier cl(&bl);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);
    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
}

TEST_CASE("BlockingTestSuite - copy_blocking_and_classification","[BlockingTestSuite]") {
    gmds::cad::FACManager geom_model;
    setUp_class(geom_model);
    gecko::blocking::Blocking bl(&geom_model, true);
    gecko::blocking::BlockingClassifier cl(&bl);

    std::set<TCellID> m_boundary_node_ids;
    std::set<TCellID> m_boundary_edge_ids;
    std::set<TCellID> m_boundary_face_ids;
    bl.extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    cl.clear_classification();
    cl.try_and_capture(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);

    auto errors = cl.detect_classification_errors();

    REQUIRE(errors.non_captured_points.size()==0);
    REQUIRE(errors.non_captured_curves.size()==0);
    REQUIRE(errors.non_captured_surfaces.size()==0);
    REQUIRE(errors.non_classified_nodes.size()==0);
    REQUIRE(errors.non_classified_edges.size()==0);
    REQUIRE(errors.non_classified_faces.size()==0);
    for (auto nId : bl.mesh().nodes()) {
        auto n = bl.mesh().get<Node>(nId);
        auto dim = bl.get_geom_dim(n);
        REQUIRE(dim == cad::GeomMeshLinker::LinkPoint);
    }
    for (auto eId : bl.mesh().edges()) {
        auto e = bl.mesh().get<Edge>(eId);
        auto dim = bl.get_geom_dim(e);
        REQUIRE(dim == cad::GeomMeshLinker::LinkCurve);
    }
    for (auto fId : bl.mesh().faces()) {
        auto f = bl.mesh().get<Face>(fId);
        auto dim = bl.get_geom_dim(f);
        REQUIRE(dim == cad::GeomMeshLinker::LinkSurface);
    }
    // for (auto bId : bl.mesh().regions()) {
    //     auto b = bl.mesh().get<Node>(bId);
    //     auto dim = bl.get_geom_dim(b);
    //     REQUIRE(dim == cad::GeomMeshLinker::LinkVolume);
    // }


    gecko::blocking::Blocking copyBl(bl);

    for (auto nId : copyBl.mesh().nodes()) {
        auto n = copyBl.mesh().get<Node>(nId);
        auto dim = copyBl.get_geom_dim(n);
        REQUIRE(dim == cad::GeomMeshLinker::LinkPoint);
    }
    for (auto eId : copyBl.mesh().edges()) {
        auto e = copyBl.mesh().get<Edge>(eId);
        auto dim = copyBl.get_geom_dim(e);
        REQUIRE(dim == cad::GeomMeshLinker::LinkCurve);
    }
    for (auto fId : copyBl.mesh().faces()) {
        auto f = copyBl.mesh().get<Face>(fId);
        auto dim = copyBl.get_geom_dim(f);
        REQUIRE(dim == cad::GeomMeshLinker::LinkSurface);
    }
    // for (auto bId : bl.mesh().regions()) {
    //     auto b = bl.mesh().get<Node>(bId);
    //     auto dim = bl.get_geom_dim(b);
    //     REQUIRE(dim == cad::GeomMeshLinker::LinkVolume);
    // }
}
