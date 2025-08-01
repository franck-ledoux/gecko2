/*----------------------------------------------------------------------------*/
#include <gecko/cblock/BlockingClassifier.h>
#include <gecko/mcts/BlockingState.h>
#include <gecko/mcts/BlockingAction.h>
#include <gmds/cad/GeomManager.h>
/*----------------------------------------------------------------------------*/
#include <vector>
/*----------------------------------------------------------------------------*/
using namespace gmds;
using namespace gecko;
using namespace gecko::cblock;
using namespace gecko::mctsc;
/*----------------------------------------------------------------------------*/
const int BlockingState::m_memory_depth = 4;
double BlockingState::weight_nodes = 10000;
double BlockingState::weight_edges = 100;
double BlockingState::weight_faces = 1;
/*----------------------------------------------------------------------------*/
BlockingState::BlockingState(std::shared_ptr<Blocking> AB, int ADepth, std::deque<double> APrevScores)
  : m_blocking(AB), m_depth(ADepth), m_memory_scores(APrevScores)
{
//	m_blocking->reset_classification();
	m_blocking->extract_boundary(m_boundary_node_ids, m_boundary_edge_ids, m_boundary_face_ids);
	BlockingClassifier(m_blocking).try_and_capture(m_boundary_node_ids,m_boundary_edge_ids, m_boundary_face_ids);
	updateMemory(computeScore());

	m_expected_optimal_score = weight_nodes * m_blocking->geom_model()->getNbPoints()
	                           + weight_edges * m_blocking->geom_model()->getNbCurves()
	                           + weight_faces * m_blocking->geom_model()->getNbSurfaces();
}
/*----------------------------------------------------------------------------*/
BlockingState::BlockingState(const BlockingState &AState) :
  m_blocking(AState.m_blocking),
  m_depth(AState.m_depth),
  m_memory_scores(AState.m_memory_scores),
  m_boundary_node_ids(AState.m_boundary_node_ids),
  m_boundary_edge_ids(AState.m_boundary_edge_ids),
m_boundary_face_ids(AState.m_boundary_face_ids),
  m_list_actions_selection(AState.m_list_actions_selection),
  m_list_actions_simulation(AState.m_list_actions_simulation)

{
}
/*----------------------------------------------------------------------------*/
BlockingState::~BlockingState()
{

}

/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::build_actions_selection() const
{
	auto actions = get_possible_cuts();
	// apply the cut on the blocking, dont use the state
	auto block_removals = get_possible_block_removals_limited();
	actions.insert(actions.end(),block_removals.begin(),block_removals.end());

	/*TODO Here we could filter equivalent actions!!!
	 * For edge cut, it means for instance, when a cut on an edge will be
	 * equivalent to the cut on a parallel edge by propagation*/
	return actions;
}
/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::get_actions_selection()
{
	if (m_list_actions_selection.empty()) {
		this->m_list_actions_selection=build_actions_selection();
	}
	return m_list_actions_selection;
}
/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::build_actions_simulation() const
{
	auto actions = get_possible_cuts_limited();
	// apply the cut on the blocking, dont use the state
	auto block_removals = get_possible_block_removals_limited();
	actions.insert(actions.end(),block_removals.begin(),block_removals.end());

	/*TODO Here we could filter equivalent actions!!!
	 * For edge cut, it means for instance, when a cut on an edge will be
	 * equivalent to the cut on a parallel edge by propagation*/
	return actions;
}
/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::get_actions_simulation()
{
	if (m_list_actions_simulation.empty()) {
		this->m_list_actions_simulation=build_actions_simulation();
	}
	return m_list_actions_simulation;
}

/*----------------------------------------------------------------------------*/
bool
BlockingState::win()
{
	/* we win if we don't have anymore classification errors. It means that the
	   state score, which is the last element of the memory scores, is equal to
	   0.*/
	return (m_memory_scores.back() == m_expected_optimal_score && m_blocking->is_valid_connected());
} /*----------------------------------------------------------------------------*/
bool
BlockingState::lost()
{
	bool val = false;
	//we lost if we don't have actions to perform
	if (!win() 	&& (get_actions_selection().empty() ||
		(m_memory_scores.size() > 1 && m_memory_scores[m_memory_scores.size()-1] < m_memory_scores[m_memory_scores.size()-2]) ||
		computeMinEdgeLenght()<0.001))
 		return true;
	return false;
	// We lost if the new score have a worst quality than the previous one
	/*if (m_memory_scores.size() > 1 && m_memory_scores[m_memory_scores.size()] < m_memory_scores[m_memory_scores.size()-1]) {
		std::cout << "memo last : " << m_memory_scores[m_memory_scores.size()] << " & memo last -1 :" << m_memory_scores[3] << std::endl;
		return true;
	}


	else
		return false;*/
/*
	// We lost if our score doesn't improve during the last steps.
	// if the memory stack is not full we keep working, so we do not lost
	if (m_memory_scores.size() < BlockingState::m_memory_depth) return false;     // not lost

	// now we check the value of the current score in comparisons to the score history
	return (m_memory_scores[4] >= m_memory_scores[3] ||
	        m_memory_scores[4] >= m_memory_scores[2] ||
	        m_memory_scores[4] >= m_memory_scores[1] ||
	        m_memory_scores[4] >= m_memory_scores[0]);*/
}
/*----------------------------------------------------------------------------*/
bool
BlockingState::draw()
{return !win() && !lost();}
/*----------------------------------------------------------------------------*/
bool
BlockingState::is_terminal()
{
	return lost() || win();
}
/*----------------------------------------------------------------------------*/
std::string
BlockingState::write(const std::string &AFileName, const int AStageIndex, const int ANodeId, const int ADepth) const
{
	return "";
}
/*----------------------------------------------------------------------------*/
double
BlockingState::computeScore()
{
	double score = computeScoreClassification();
	return score;

}
/*----------------------------------------------------------------------------*/
double
BlockingState::computeScoreClassification()
{
	auto errors = BlockingClassifier(m_blocking).detect_classification_errors();

	return weight_nodes*(m_blocking->geom_model()->getNbPoints()-errors.non_captured_points.size())+
			 weight_edges*(m_blocking->geom_model()->getNbCurves()-errors.non_captured_curves.size())+
			 weight_faces*(m_blocking->geom_model()->getNbSurfaces()-errors.non_captured_surfaces.size());
}
/*----------------------------------------------------------------------------*/
double
BlockingState::computeScoreQuality()
{
	double quality = 0;

}
/*----------------------------------------------------------------------------*/
double
BlockingState::computeMinEdgeLenght() const
{
	double minusEdge = 1000;
	auto listEdges= m_blocking->get_all_edges();

	for(auto edge : listEdges){
		auto pointsCurrentEdge =m_blocking->get_nodes_of_edge(edge);
		double edgeLength = gmds::math::Segment(pointsCurrentEdge[0]->info().point, pointsCurrentEdge[1]->info().point).computeLength();
		if(edgeLength<minusEdge){
			minusEdge=edgeLength;
		}
	}

	return minusEdge;

}
/*----------------------------------------------------------------------------*/
void
BlockingState::updateMemory(double AScore)
{
	// We add a new score in the memory stack
	// the oldest score is in 0, the newest is in 3
	if (m_memory_scores.size() == 4) {
		// the stack is full we remove the first element
		for (int i = 0 ; i <3;i++){
			m_memory_scores[i] = m_memory_scores[i+1];
		}
		m_memory_scores[4]= AScore;
	}
	// we add the new score at the back
	m_memory_scores.push_back(AScore);
}
/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::get_possible_block_removals() const
{
	auto nodes = m_blocking->get_all_nodes();
	std::set<TCellID> blocks_to_keep;
	for (auto n : nodes) {
		if (n->info().geom_dim == 0) {
			// n is classified onto a geom point
			// if it belongs to a single block, the corresponding block cannot
			// be removed
			auto n_blocks = m_blocking->get_blocks_of_node(n);
			if (n_blocks.size() == 1) {
				blocks_to_keep.insert(n_blocks[0]->info().topo_id);
			}
		}
	}
	std::vector<std::shared_ptr<IAction> > actions;
	// identify blocks with their centroid inside the geometry
	cad::GeomManager *geom = m_blocking->geom_model();

	/*
	auto blocks = m_blocking->get_all_blocks();
	for (auto b : blocks) {
		gmds::math::Point pt = m_blocking->get_center_of_block(b);
		bool is_inside = geom->is_in(pt);
		if(is_inside && without_blocks_in) {
			blocks_to_keep.insert(m_blocking->get_block_id(b));
		}
	}*/

	// all the other blocks can be removed
	auto all_blocks = m_blocking->get_all_id_blocks();
	for (auto b : all_blocks) {
		//We check if b is a block to keep?
		if (blocks_to_keep.find(b) == blocks_to_keep.end()){
			// b is not to keep, we remove it
			actions.push_back(std::make_shared<BlockRemovalAction>(b));
		}
	}

	return  actions;
}
/*----------------------------------------------------------------------------*/

std::vector<std::shared_ptr<IAction>>
BlockingState::get_possible_block_removals_limited() const
{
	auto nodes = m_blocking->get_all_nodes();
	std::set<TCellID> blocks_to_keep;
	for (auto n : nodes) {
		if (n->info().geom_dim == 0) {
			// n is classified onto a geom point
			// if it belongs to a single block, the corresponding block cannot
			// be removed
			auto n_blocks = m_blocking->get_blocks_of_node(n);
			if (n_blocks.size() == 1) {
				blocks_to_keep.insert(n_blocks[0]->info().topo_id);
			}
		}
	}
	std::vector<std::shared_ptr<IAction> > actions;
	// identify blocks with their centroid inside the geometry
	cad::GeomManager *geom = m_blocking->geom_model();

	auto blocks = m_blocking->get_all_blocks();
	for (auto b : blocks) {
		gmds::math::Point pt = m_blocking->get_center_of_block(b);

		bool is_inside = geom->getVolume(1)->isIn(pt);
		if(is_inside) {
			blocks_to_keep.insert(m_blocking->get_block_id(b));
		}
	}

	// all the other blocks can be removed
	auto all_blocks = m_blocking->get_all_id_blocks();
	for (auto b : all_blocks) {
		//We check if b is a block to keep?
		if (blocks_to_keep.find(b) == blocks_to_keep.end()){
			// b is not to keep, we remove it
			actions.push_back(std::make_shared<BlockRemovalAction>(b));
		}
	}

	return  actions;
}
/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::get_possible_cuts() const
{
	double epsilon = 1e-2;
	std::set<std::pair<gmds::math::Point,std::pair<TCellID, double>>> list_cuts;
	// We look which geometric points and curves are not yet captured
	auto non_captured_entities = BlockingClassifier(m_blocking).detect_classification_errors();
	auto non_captured_points = non_captured_entities.non_captured_points;
	auto non_captured_curves = non_captured_entities.non_captured_curves;

	auto all_block_edges = m_blocking->get_all_edges();
	for (auto p : non_captured_points) {
		// the point p is not captured. We look for a cut along a block edge to capture it
		auto [edge_cut, param_cut,dist_cut]  = m_blocking->get_cut_info(p,all_block_edges);
		// We only consider cut of edge that occur inside the edge and not on one of
		// its end points (so for param O or 1)
		if (epsilon < param_cut  && param_cut <1.0-epsilon) {
			auto e2cut = edge_cut;
			gmds::math::Point pointCapt = m_blocking->geom_model()->getPoint(p)->point();
		    list_cuts.insert(std::make_pair(pointCapt,std::make_pair(e2cut->info().topo_id,param_cut)));
		    // TODO: verify if we always have a cut that is possible for a point???
		    // As we cut the edge std::get<0>(action)->info().topo_id, we remove it to avoid multiple actions on the same edge
		    /*std::vector<Blocking::Edge> edges_to_remove;
		    m_blocking->get_all_sheet_edges(e2cut,edges_to_remove);


			// Loop through each element to delete
			for (auto e : edges_to_remove) {
				// Find the element in the vector
				auto it = find(all_block_edges.begin(), all_block_edges.end(), e);
				// If the element is found, erase it
				if (it != all_block_edges.end()) {
					all_block_edges.erase(it);
				}
			}*/
		}
	}

	for (auto c_id : non_captured_curves) {
		auto c = m_blocking->geom_model()->getCurve(c_id);
		gmds::TCoord minXYX[3];
		gmds::TCoord maxXYX[3];
		c->computeBoundingBox(minXYX, maxXYX);

		math::Point bb_corners[8]={
			math::Point(minXYX[0], minXYX[1], minXYX[2]),
			math::Point(maxXYX[0], maxXYX[1], maxXYX[2]),
			math::Point(minXYX[0], minXYX[1], maxXYX[2]),
			math::Point(maxXYX[0], minXYX[1], maxXYX[2]),
			math::Point(maxXYX[0], minXYX[1], minXYX[2]),
			math::Point(minXYX[0], maxXYX[1], maxXYX[2]),
			math::Point(minXYX[0], maxXYX[1], minXYX[2]),
			math::Point(maxXYX[0], maxXYX[1], minXYX[2])
		};
		for (auto p:bb_corners) {
			auto cut_info_p = m_blocking->get_cut_info(p, all_block_edges);
			auto param_cut = std::get<1>(cut_info_p);
			// We only consider cut of edge that occur inside the edge and not on one of
			// its end points (so for param O or 1)
			if (epsilon < param_cut  && param_cut < 1.0-epsilon) {
				auto e2cut = std::get<0>(cut_info_p);
				list_cuts.insert(std::make_pair(p,std::make_pair(e2cut->info().topo_id, std::get<1>(cut_info_p))));
				// TODO: verify if we always have a cut that is possible for a point???
				// As we cut the edge std::get<0>(action)->info().topo_id, we remove it to avoid multiple actions on the same edge
				/*std::vector<Blocking::Edge> edges_to_remove;
				m_blocking->get_all_sheet_edges(e2cut,edges_to_remove);

				// Loop through each element to delete
				for (auto e : edges_to_remove) {
					// Find the element in the vector
					auto it = find(all_block_edges.begin(), all_block_edges.end(), e);
					// If the element is found, erase it
					if (it != all_block_edges.end()) {
						all_block_edges.erase(it);
					}
				}*/
			}

		}
	}

	std::vector<std::shared_ptr<IAction> > all_actions;
	for(auto cut_info:list_cuts)
		all_actions.push_back(std::make_shared<EdgeCutAction>(cut_info.second.first,cut_info.second.second,cut_info.first));

	std::vector<std::shared_ptr<Blocking>> list_blockings;
	std::vector<std::shared_ptr<IAction> > filtered_actions;

	for (auto a : all_actions) {
		auto currentState = std::make_shared<BlockingState>(*this);
		auto new_state = std::dynamic_pointer_cast<BlockingState>(a->apply_on(currentState));
		auto current_blocking = new_state->get_blocking();
		if (list_blockings.empty()) {
			list_blockings.push_back(current_blocking);
			filtered_actions.push_back(a);
		}
		else {
			bool find = false;
			for (auto b : list_blockings) {
				if (*b == *current_blocking) {
					find = true;
					break;
				}
			}
			if (!find) {
				list_blockings.push_back(current_blocking);
				filtered_actions.push_back(a);
			}
		}
	}
	return filtered_actions;
}

/*----------------------------------------------------------------------------*/
std::vector<std::shared_ptr<IAction>>
BlockingState::get_possible_cuts_limited() const
{
	double epsilon = 1e-2;
	std::set<std::pair<gmds::math::Point,std::pair<TCellID, double>>> list_cuts;
	// We look which geometric points and curves are not yet captured
	auto non_captured_entities = BlockingClassifier(m_blocking.get()).detect_classification_errors();
	auto non_captured_points = non_captured_entities.non_captured_points;
	auto non_captured_curves = non_captured_entities.non_captured_curves;

	auto all_block_edges = m_blocking->get_all_edges();
	for (auto p : non_captured_points) {
		// the point p is not captured. We look for a cut along a block edge to capture it
		auto action = m_blocking->get_cut_info(p,all_block_edges);
		auto param_cut = std::get<1>(action);
		// We only consider cut of edge that occur inside the edge and not on one of
		// its end points (so for param O or 1)
		if (epsilon < param_cut  && param_cut <1.0-epsilon) {
			auto e2cut = std::get<0>(action);
			gmds::math::Point pointCapt = m_blocking->geom_model()->getPoint(p)->point();
		    list_cuts.insert(std::make_pair(pointCapt,std::make_pair(e2cut->info().topo_id, std::get<1>(action))));
		    // TODO: verify if we always have a cut that is possible for a point???
		    // As we cut the edge std::get<0>(action)->info().topo_id, we remove it to avoid multiple actions on the same edge
		    std::vector<Blocking::Edge> edges_to_remove;
		    m_blocking->get_all_sheet_edges(e2cut,edges_to_remove);


			// Loop through each element to delete
			for (auto e : edges_to_remove) {
				// Find the element in the vector
				auto it = find(all_block_edges.begin(), all_block_edges.end(), e);
				// If the element is found, erase it
				if (it != all_block_edges.end()) {
					all_block_edges.erase(it);
				}
			}
		}
	}

	for (auto c_id : non_captured_curves) {
		auto c = m_blocking->geom_model()->getCurve(c_id);
		gmds::TCoord minXYX[3];
		gmds::TCoord maxXYX[3];
		c->computeBoundingBox(minXYX, maxXYX);

		math::Point bb_corners[8]={
			math::Point(minXYX[0], minXYX[1], minXYX[2]),
			math::Point(maxXYX[0], maxXYX[1], maxXYX[2]),
			math::Point(minXYX[0], minXYX[1], maxXYX[2]),
			math::Point(maxXYX[0], minXYX[1], maxXYX[2]),
			math::Point(maxXYX[0], minXYX[1], minXYX[2]),
			math::Point(minXYX[0], maxXYX[1], maxXYX[2]),
			math::Point(minXYX[0], maxXYX[1], minXYX[2]),
			math::Point(maxXYX[0], maxXYX[1], minXYX[2])
		};
		for (auto p:bb_corners) {
			auto cut_info_p = m_blocking->get_cut_info(p, all_block_edges);
			auto param_cut = std::get<1>(cut_info_p);
			// We only consider cut of edge that occur inside the edge and not on one of
			// its end points (so for param O or 1)
			if (epsilon < param_cut  && param_cut < 1.0-epsilon) {
				auto e2cut = std::get<0>(cut_info_p);
				list_cuts.insert(std::make_pair(p,std::make_pair(e2cut->info().topo_id, std::get<1>(cut_info_p))));
				// TODO: verify if we always have a cut that is possible for a point???
				// As we cut the edge std::get<0>(action)->info().topo_id, we remove it to avoid multiple actions on the same edge
				std::vector<Blocking::Edge> edges_to_remove;
				m_blocking->get_all_sheet_edges(e2cut,edges_to_remove);

				// Loop through each element to delete
				for (auto e : edges_to_remove) {
					// Find the element in the vector
					auto it = find(all_block_edges.begin(), all_block_edges.end(), e);
					// If the element is found, erase it
					if (it != all_block_edges.end()) {
						all_block_edges.erase(it);
					}
				}
			}

		}
	}

	std::vector<std::shared_ptr<IAction> > actions;
	for(auto cut_info:list_cuts)
		actions.push_back(std::make_shared<EdgeCutAction>(cut_info.second.first,cut_info.second.second,cut_info.first));

	return actions;
}