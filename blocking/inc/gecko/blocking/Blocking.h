/*----------------------------------------------------------------------------*/
#ifndef GECKO_BLOCKING_BLOCKING_H_
#define GECKO_BLOCKING_BLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <string>
#include <tuple>

#include <gmds/cad/GeomManager.h>
#include <gmds/cad/GeomMeshLinker.h>
#include <gmds/ig/Mesh.h>
#include <gmds/math/Point.h>
#include <gmds/utils/CommonTypes.h>
#include <gmds/utils/Exception.h>

using namespace gmds;
/*----------------------------------------------------------------------------*/
namespace gecko {
namespace blocking {

/**@struct CellInfo
 * @brief This structure gather the pieces of data that are shared by any
 * 		 blocking cell. Each cell is defined by:
 * 		 	- its dimension @p topo_dim, which is 0 for a node, 1 for an edge, 2
 * 		     for a face, and 3 for a block
 * 		   - its id @p topo_id, which is unique. Each time a cell is created, the
 * 		  blocking structure assigns an id to it.
 * 		   - The data relative to the geometric cell it is classified on. A
 * 		  geometrical cell is defined by the couple (@p geom_dim, @p geom_id).
 * 		  A value of 4 for @p geom_dim means that the block cell is not classified.
 */
struct CellInfo
{
	/*** dimension of the topological cell */
	int topo_dim;
	/*** unique id of the topological cell */
	int topo_id;
	/*** link to the cad manager to have access to geometric cells */
	cad::GeomManager *geom_manager;
	/*** dimension of the geometrical cell we are classified on */
	int geom_dim;
	/*** unique id of the geometrical cell */
	int geom_id;

	/** @brief Constructor
	 * @param Ac the id counter; the CGAL cmap copy constructor requires a CellInfo()
	 *           call with no params
	 * @param AManager the geometric manager to access cells
	 * @param ATopoDim Cell dimension
	 * @param AGeomDim on-classify geometric cell dimension (4 if not classified)
	 * @param AGeomId on-classify geometric cell unique id
	 */
	CellInfo(cad::GeomManager *AManager = nullptr, const int ATopoDim = 4, const int AGeomDim = 4, const int AGeomId = NullID) :
	  topo_dim(ATopoDim), geom_manager(AManager), geom_dim(AGeomDim), geom_id(AGeomId)
	{

	}
};
/*----------------------------------------------------------------------------*/
/**@struct NodeInfo
 * @brief Specific structure for describing a node. It extends the "CellInfo"
 *        structure by giving a location (x,y,z).
 */
struct NodeInfo : CellInfo
{
	/*** node location in space, i.e. a single point */
	math::Point point;
	/** @brief Constructor
	 * @param Ac the id counter; the CGAL cmap copy constructor requires a CellInfo()
	 *           call with no params
	 * @param AManager the geometric manager to access cells
	 * @param AGeomDim on-classify geometric cell dimension (4 if not classified)
	 * @param AGeomId  on-classify geometric cell unique id
	 * @param APoint   geometric location
	 */
	NodeInfo(cad::GeomManager *AManager = nullptr,
	         const int AGeomDim = 4,
	         const int AGeomId = NullID,
	         const math::Point &APoint = math::Point(0, 0, 0)) :
	  CellInfo(AManager, 0, AGeomDim, AGeomId), point(APoint)
	{
	}
};
/*----------------------------------------------------------------------------*/
/** @struct MergeFunctor
 * @brief This structure provides a function for the Gmap class in order to
 * handle attributes when i-cells are merged. For instance, when two blocks B1
 * and B2 are glued along the face F1 for B1 and F2 for B2, those faces are merged
 * into a single one. Attributes of faces F1 and F2 are so merged too (it will
 * also be the case of the nodes and edges that are incident to faces F1 and F2).
 * common face.
 *
 * This functor defines the behavior for the attributes of Edges, Faces and Blocks.
 * More specifically, when two i-cells are merged, we consider the first one as the
 * master and the second one as the slave. The resulting i-cell keep the topological
 * data of the first one.
 *
 * For the geometrical data, we look at the lowest dimension of the associated geometrical
 * cell. We are always constraint to preserving classification of lowest dimensional cell.
 * For instance, if an edge E1, classified on a Surface, is merged with an edge E2, classified
 * on a curve, we preserve the curve classification.
 */
struct MergeFunctor
{
	/** @brief Merging function for general cells (not a node).
	 * @tparam Cell_attribute
	 * @param[in] ACA1 the first attribute
	 * @param[in] ACA2 the second attribute
	 */
	template<class Cell_attribute> void operator()(Cell_attribute &ACA1, Cell_attribute &ACA2)
	{
		if (ACA1.info().geom_dim == ACA2.info().geom_dim) {
			// the cells are classifed on the same dim geom entity
			if (ACA1.info().geom_id == ACA2.info().geom_id) {
				ACA1.info().geom_dim = ACA2.info().geom_dim;
			}
			else
				throw GMDSException("Classification error!!!");
		}
		else if (ACA1.info().geom_dim < ACA2.info().geom_dim) {
			// the cells are classifed on the same dim geom entity
			ACA1.info().geom_dim = ACA1.info().geom_dim;
			ACA1.info().geom_id = ACA1.info().geom_id;
		}
		else {     // third case: ca1.info().dim>ca2.info().dim
			ACA1.info().geom_dim = ACA2.info().geom_dim;
			ACA1.info().geom_id = ACA2.info().geom_id;
		}
		// We keep the topological characteristics of the ACA1 cell
		ACA1.info().topo_dim = ACA1.info().topo_dim;
		ACA1.info().topo_id = ACA1.info().topo_id;
	}
};
/*----------------------------------------------------------------------------*/
/** @struct MergeFunctorNode
 * @brief This structure provides a function for the Gmap class in order to
 * handle attributes when O-cells are merged. For instance, when two blocks B1
 * and B2 are glued along the face F1 for B1 and F2 for B2, those faces are merged
 * into a single one. Incident nodes of F1 and F2 are then merged too.
 *
 * This functor defines the behavior when merging the attributes of nodes.
 *  The resulting attribute keep the topological data of the first one.
 *
 * For the geometrical data, we look at the lowest dimension of the associated geometrical
 * cell. We are always constraint to preserving classification of lowest dimensional cell.
 * For instance, if a node N1, classified on a point, is merged with a node N2, classified
 * on a surface, we preserve the point classification.
 *
 * The node location is also impacted. If the merged nodes are on the same geometrical
 * cell, we take the average location on the geometrical entity. Otherwise, we keep the
 * most constrained location
 */
struct MergeFunctorNode
{

	/** @brief Merging function for nodes
	 * @tparam Cell_attribute
	 * @param[in/out] ACA1 the first node attribute
	 * @param[in] ACA2 the second node attribute
	 */
	template<class Cell_attribute> void operator()(Cell_attribute &ACA1, Cell_attribute &ACA2)
	{
		if (ACA1.info().geom_dim == ACA2.info().geom_dim) {
			if (ACA1.info().geom_id == ACA2.info().geom_id) {
				// nodes are clasified on the same geometrical entity!
				ACA1.info().point = 0.5 * (ACA1.info().point + ACA2.info().point);
				// projection on the geom entity
				cad::GeomEntity *geom_cell = ACA1.info().geom_manager->getEntity(ACA1.info().geom_id, ACA1.info().geom_dim);
				if (geom_cell != nullptr) ACA1.info().point = geom_cell->closestPoint(ACA1.info().point);
			}
			else
				throw GMDSException("Classification error!!!");
		}
		else if (ACA1.info().geom_dim < ACA2.info().geom_dim) {
			// the cells are classifed on the same dim geom entity
			ACA1.info().geom_dim = ACA1.info().geom_dim;
			ACA1.info().geom_id = ACA1.info().geom_id;
			ACA1.info().point = ACA1.info().point;
		}
		else {     // third case: ca1.info().dim>ca2.info().dim
			ACA1.info().geom_dim = ACA2.info().geom_dim;
			ACA1.info().geom_id = ACA2.info().geom_id;
			ACA1.info().point = ACA2.info().point;
		}
		// We keep the topological characteristics of the ACA1 cell
		ACA1.info().topo_dim = ACA1.info().topo_dim;
		ACA1.info().topo_id = ACA1.info().topo_id;
	}
};
/*----------------------------------------------------------------------------*/
/** @struct SplitFunctor
 * @brief This structure provides a function for the Gmap class in order to
 * handle attributes when i-cells are split. It simply consists in duplicating
 * the attributes.
 */
struct SplitFunctor
{

	/** @brief Splitting function for general cells (including nodes).
	 * @tparam Cell_attribute
	 * @param[in] ACA1 the first cell attribute
	 * @param[in] ACA2 the second cell attribute
	 */
	template<class Cell_attribute> void operator()(Cell_attribute &ca1, Cell_attribute &ca2)
	{
		ca1.info().geom_dim = ca1.info().geom_dim;
		ca1.info().geom_id = ca1.info().geom_id;

		ca2.info().geom_dim = ca1.info().geom_dim;
		ca2.info().geom_id = ca1.info().geom_id;
		ca2.info().topo_dim = ca1.info().topo_dim;
		ca2.info().topo_id = ca1.info().counter->get_and_increment_id();
	}
};

/*----------------------------------------------------------------------------*/
/** @struct SplitFunctorNode
 * @brief This structure provides a function for the Gmap class in order to
 * handle attributes when nodes are split. It simply consists in duplicating
 * the attributes.
 */
struct SplitFunctorNode
{
	template<class Cell_attribute> void operator()(Cell_attribute &ca1, Cell_attribute &ca2)
	{
		ca2.info().geom_dim = ca1.info().geom_dim;
		ca2.info().geom_id = ca1.info().geom_id;
		ca2.info().point = ca1.info().point;
		ca2.info().topo_dim = ca1.info().topo_dim;
		ca2.info().topo_id = ca1.info().counter->get_and_increment_id();
	}
};

/*----------------------------------------------------------------------------*/
/**@class Blocking
 * @brief Provide a curved blocking data structure using the 3-G-Map model
 * 		 as described and provided by CGAL.
 * 		 (see https://doc.cgal.org/latest/Generalized_map/index.html)
 */
class  Blocking
{
 public:
	using Block = gmds::Region;
	using Face = gmds::Face;
	using Edge = gmds::Edge;
	using Node =gmds::Node;
	/** @brief Constructor that takes a geom model as an input. A
	 * blocking is always used for partitioning a geometric domain.
	 * @param[in] AGeomModel the geometric model we want to block
	 * @param[in] AInitAsBoundingBox indicates that the block structure must remain
	 * empty (false) or be initialized as the bounding box of @p AGeomModel
	 */
	Blocking(cad::GeomManager *AGeomModel, bool AInitAsBoundingBox = false);

	Blocking(const Blocking &ABl);


	/** @brief  Destructor
	 */
	virtual ~Blocking();

	/*------------------------------------------------------------------------*/
	/** \brief Overide operator==. It is topology-based.
		 *
		 * \param ABlocking a blocking
	 */
	bool operator==(Blocking& ABlocking);

	void display_info();
	/**================================================================
	 *  QUERIES OPERATIONS
	 *=================================================================*/

	/**@brief gives access to the associated geom model
	 * @return the internal geom model
	 */
	cad::GeomManager *geom_model() const;

	Mesh& mesh(){return m_mesh;}

	cad::GeomMeshLinker::eLink get_geom_dim(Node& AN);
	cad::GeomMeshLinker::eLink get_geom_dim(Edge& AE);
	cad::GeomMeshLinker::eLink get_geom_dim(Face& AF);
	cad::GeomMeshLinker::eLink get_geom_dim(Region& AR);

	int get_geom_id(Node& AN);
	int get_geom_id(Edge& AE);
	int get_geom_id(Face& AF);
	int get_geom_id(Region& AR);

	void set_geom_link(Node& AN,cad::GeomMeshLinker::eLink ADim, int AnId);
	void set_geom_link(Edge& AE,cad::GeomMeshLinker::eLink ADim, int AnId);
	void set_geom_link(Face& AF,cad::GeomMeshLinker::eLink ADim, int AnId);
	void set_geom_link(Region& AR,cad::GeomMeshLinker::eLink ADim, int AnId);

	/**@brief We get the id lists of nodes, edges and faces that make
	 * the blocking boundary
	 *
	 * @param[out] ANodeIds ids of boundary nodes
	 * @param[out] AEdgeIds ids of boundary edges
	 * @param[out] AFaceIds ids of boundary faces
	 */
	void extract_boundary(std::set<TCellID> &ANodeIds, std::set<TCellID> &AEdgeIds, std::set<TCellID> &AFaceIds);

	/**@brief This method reset all the nodes, edges and faces classification data to geom dim -1, geom id 4
	 */
	void reset_classification();

	/**@brief Get all the parallel edges composing the sheet defined from edge @p AE
	* @param[in]  AE		the edge we start from
	* @param[in]  AE_firstN the fist node of AE we orient AE from
	 * @param[out] AEdges	all the edges of the sheet defined by @p AE
	 * @param[out] AE2N		the orient edges - for each edge of AE we, get the nodes in the same direction
	 * @param[out] ABlocks	all the blocks of the sheet defined by @p AE
	 */
	void get_sheet_cells(const Edge AE,
		const Node AE_fistN,
		std::vector<Edge> &AEdges,
		std::map<TCellID,std::pair<TCellID,TCellID>>& AE2N,
		std::vector<Block> &ABlocks);

	/**@brief Get all the parallel edges composing sheets. Each set of parallel
	 * 		 edges is an item pf @p ASheetEdges
	 * return all the edges gathered by sheet
	 */
	std::vector<std::vector<Edge>> get_all_sheet_edge_sets();

	void get_all_sheet_edges(const Edge AE, std::vector<Edge> &returnEdges);

	/**@brief Get all the blocks of the chord defined from face @p AF. All the returned darts
	 * belong to different blocks.
	 * @param[in]  AF			the face we start from
	 * @param[out] ADarts	one dart per face of the sheet defined by @p AE
	 */
	std::vector<Block> get_all_chord_blocks(const Face AF);

	/**\brief return the parameters for do the cut_sheet. It returns the closest edge, which can remain quite far.
	 * @param[in] pointId 		A point id
	 * @return return the parameters for the cut, we get the edge (first), the parameter included in ]0,1[(second),
	 * and the distance to the edge (third)
	 */
	std::tuple<Blocking::Edge, double, double> get_cut_info(const int APointId,
	                                                        const std::vector<Blocking::Edge> AEdges);

	/**\brief return the parameters for doing the cut_sheet. It returns the closest edge, which can remain quite far.
	 * @param[in] APoint 		A math point
	 * @return return the parameters for the cut, we get the edge (first) and the parameter included in ]0,1[ (second),
	 * and the distance to the edge (third)
	 */
	std::tuple<Blocking::Edge, double, double> get_cut_info(const gmds::math::Point &APoint,
	                                                        const std::vector<Blocking::Edge> AEdges);


	bool is_valid_connected();

	Edge get_edge(const TCellID ANodeId0, const TCellID ANodeId1);

	std::vector<Blocking::Block> getBlocks(const Blocking::Node ANode);
	std::vector<Blocking::Block> getBlocks(const TCellID ANodeId);

	/**================================================================
	 *  GEOMETRIC OPERATIONS
	 *=================================================================*/

	/**@brief moves node @p AN towards the expected new location @p ALoc.
	 * If @p AN is classified onto a geometrical cell, the node @p AN
	 * is first moved to @p ALoc, then it is projected onto the
	 * geometrical cell it is classified on.
	 *
	 * @param AN the node to move
	 * @param ALoc the new node location
	 */
	void move_node(Node AN, math::Point &ALoc);

	/**================================================================
	 *  TOPOLOGICAL OPERATIONS THAT MODIFY ALSO GEOMETRY
	 *=================================================================*/
	/** Create a single hexahedral block in the blocking structure
	 * @return The created block
	 */
	Block create_block(const math::Point &AP1,
	                   const math::Point &AP2,
	                   const math::Point &AP3,
	                   const math::Point &AP4,
	                   const math::Point &AP5,
	                   const math::Point &AP6,
	                   const math::Point &AP7,
	                   const math::Point &AP8);

	/** Removes the block @AB from the structure
	 * @param[in] ABlockId the block id to remove
	 */
	void remove_block(const gmds::TCellID ABlockId);

	/** Removes the block @AB from the structure
	 * @param[in] AB the block to remove
	 */
	void remove_block(Block AB);
	/**@brief Split the sheet defined by edge @p AE at the closest position of @p AP.
	 * 		 The method project @p AP on @p AE. It gives us a cut ratio, we use on
	 * 		 all the parallel edges.
	 * @param[in] AE an edge we want to split in two edges
	 * @param[in] AP a point we use to define where AE must be cut.
	 */
	void cut_sheet(const Edge AE, const Node AN, const math::Point &AP);
	/**@brief Split the sheet defined by edge @p AE at the parameter @p AParam, which is included
	 * 		 in ]0,1[. The end point of @p AE with the lowest id is at parameter 0,
	 * 		 the second one at parameter 1.
	 *
	 * @param[in] AE an edge we want to split in two edges
	 * @param[in] AParam a parameter included in ]0,1[
	 */
	void cut_sheet(const Edge AE, const Node AN,  const double AParam);

	/**@brief Split the sheet defined by edge @p AnEdgeId at the parameter @p AParam, which is included
	 * 		 in ]0,1[. The first end point of @p AE is at parameter 0, the second one at parameter 1.
	 *
	 * @param[in] AnEdgeId an edge id we want to split in two edges
	 * @param[in] AParam a parameter included in ]0,1[
	 */
	void cut_sheet(const TCellID AnEdgeId, const TCellID ANodeId,  const double AParam);

	/**@brief Split the sheet defined by edge @p AE
	 * @param[in] AE an edge we want to split in two edges
	 */
	void cut_sheet(const Edge AE);

	/**@brief Split the sheet defined by edge @p AnEdgeId at the parameter @p AParam, which is included
	 * 		 in ]0,1[. We will select the first end point of @p AE as a starting point for the cut and the application of the @p AParam.
	 * @param[in] AnEdgeId an edge id we want to split in two edges
	 * * @param[in] AParam a parameter included in ]0,1[
	 */
	void cut_sheet(const TCellID AnEdgeId, const double AParam);

	/**@brief Collapse the chord starting from face @p AF by merging
	 * nodes @p AN1 and @p AN2. The operation is possible if and only if:
	 * - @p AN1 and @p AN2 are opposite nodes in face @p AF.
	 * - the chord defined from @p AF is simple, i.e. it doesn't intersect itself
	 * - Classification doesn't prevent the collapse
	 * @param[in] AE an edge we want to split in two edges
	 * @return true if the operation succeeded, false if the operation
	 */
	//bool collapse_chord(const Face AF, const Node AN1, const Node AN2);

	/**@brief Indicates if the set of faces provided in @p AFaces is
	 * a manifold surface that splits the mesh in two pillow-compatible
	 * regions
	 * @param[in] AFaces a set of faces
	 * @return true if we have a manifold surface
	 */
	//bool validate_pillowing_surface(std::vector<Face> &AFaces);

	/**@brief Pillow the set of faces @p AFaces.
	 * @param[in] AFaces a set of faces
	 * @return true if we have a manifold surface, false otherwise.
	 */
	//bool pillow(std::vector<Face> &AFaces);

	/**@brief Smooth the classified block structure along curves.
	 * Simple implementation using a Laplacian-kind approach.
	 * @param[in] ANbIterations number of smoothing stages along curves
	 */
	void smooth_curves(const int ANbIterations);
	/**@brief Smooth the classified block structure on surfaces.
	 * Simple implementation using a Laplacian-kind approach.
	 * @param[in] ANbIterations number of smoothing stages on surfaces
	 */
	void smooth_surfaces(const int ANbIterations);
	/**@brief Smooth the classified block structure inside the volume
	 * Simple implementation using a Laplacian-kind approach.
	 * @param[in] ANbIterations number of smoothing stages
	 */
	void smooth_volumes(const int ANbIterations);
	/**@brief Smooth the whole classified block structure. Simple implementation using a Laplacian-kind approach.
	 * @param[in] ANbIterations number of smoothing stages in each dimension (curves, surfaces, volume)
	 */
	void smooth(const int ANbIterations);

	/**@brief intiialize the block structure from a gecko cellular mesh. The provided
	 * 		 mesh @p ACellMesh must have the following characteristics:
	 * 		 - DIM3, N, E, F, R, R2N, F2N, E2N
	 *
	 * @param[in,out] ACellMesh A cellular mesh
	 */
	void init_from_mesh(Mesh& ACellMesh);


	/**@brief intiialize the block structure from the bounding box of the geom model
	 */
	void init_from_bounding_box();


	/**\brief save the blocking on vtk. During the process, the curved blocking is convert on a mesh.
	 * @param[in] AFileName 		the name used for the file
	 */
	void save_vtk_blocking(const std::string &AFileName);
 private:
	/**\brief Order the edges of @p AEdges accordingly to their distance to @p AP.
	 * 		 More specifically, we orthogonaly project @p AP on each edge of @p AEdges.
	 * 		 For each edge, we store the distance between @p AP and the edge and the coordinate
	 * 		 the projected point inside the edge (between 0 and 1). This method is internal and
	 * 		 use by get_cut_info
	 * @param[in] AP 		A Point to project on each edge
	 * @param[in] AEdges 	A set of edges
	 * @return for each edge, we get the distance (first) and the coordinate (second)
	 */
	std::vector<std::pair<double, double>> get_projection_info(const math::Point &AP, std::vector<Blocking::Edge> &AEdges);

	/**\brief Project a point orthogonally onto a block edge. This method is internal and
	 * 		 use by get_projection_info
	 * @param[in] AP 		A Point to project on each edge
	 * @param[in] AEdge 	An edge
	 * @return we get the distance (first) and the coordinate (second)
	 */
	std::pair<double, double> get_projection_info(const math::Point &AP, Blocking::Edge &AEdge);

	std::tuple<TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID, TCellID>
	get_parallel_edges(Block AB, Edge AE, Node AE_first);
 private:
	/*** the associated geometric model*/
	cad::GeomManager *m_geom_model;
	/*** the underlying n-g-map model*/
	gmds::Mesh m_mesh;
	gmds::cad::GeomMeshLinker m_mesh_linker;
};
/*----------------------------------------------------------------------------*/
}     // namespace mctsblock
/*----------------------------------------------------------------------------*/
}     // namespace gecko
/*----------------------------------------------------------------------------*/
#endif     // GECKO_GBLOCK_BLOCKING_H_
