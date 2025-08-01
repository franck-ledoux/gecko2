/*---------------------------------------------------------------------------*/
#include <chrono>
#include <iostream>
#include <random>
#include <fstream>
#include <nlohmann/json.hpp>
/*---------------------------------------------------------------------------*/
#include <gecko/mcts/MCTSAgent.h>
#include <gecko/mcts/BlockingState.h>
/*---------------------------------------------------------------------------*/
using json = nlohmann::json;
/*---------------------------------------------------------------------------*/
MCTSAgent::MCTSAgent(const IRewardFunction* ARewardFunction,
                     const ISelectionFunction* ASelectFunction,
                     const int AMaxIterations,
                     const int AMaxSeconds,
                     const int AMaxSimulationDepth)
: m_tree(nullptr),
  m_reward_function(ARewardFunction),
  m_select_function(ASelectFunction),
  m_max_iterations(AMaxIterations),
  m_max_seconds(AMaxSeconds),
  m_simulation_depth(AMaxSimulationDepth),
  m_debug_activate(false),
  m_debug_file_prefix("mcts"),
  m_debug_mode(MCTSAgent::OUT_END_ONLY),
  m_debug_frequency(1)
{}
/*---------------------------------------------------------------------------*/
MCTSAgent::~MCTSAgent() {
    delete m_tree;
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<IState> MCTSAgent::get_best_solution() {
    const MCTSTree* node = m_tree;
    while (!node->is_terminal() && node->has_children()){
        node=node->get_most_visited_child();
    }
    return node->get_state();
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<IState> MCTSAgent::get_best_winning_solution()
{	 const MCTSTree* node = m_tree;
	 while (node != NULL && !node->is_terminal() && node->has_children()){
		  node=node->get_most_winning_child();
	 }
	 return node->get_state();
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<IState> MCTSAgent::get_most_visited_child() {
    const MCTSTree* node = m_tree;

    if (!node->is_terminal() && node->has_children()){
        node=node->get_most_visited_child();
    }
    return node->get_state();
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<IState> MCTSAgent::get_most_winning_child() {
	 const MCTSTree* node = m_tree;

	 if (!node->is_terminal() && node->has_children()){
		  node=node->get_most_winning_child();
	 }
	 return node->get_state();
}
/*---------------------------------------------------------------------------*/
MCTSTree *MCTSAgent::get_most_winning_node() {
	MCTSTree* node = m_tree;

	if (!node->is_terminal() && node->has_children()){
		node=node->get_most_winning_child();
	}
	return node;
}

/*---------------------------------------------------------------------------*/
MCTSTree* MCTSAgent::expand(MCTSTree* ANode) {
    if(!ANode->is_fully_expanded() && !ANode->is_terminal())
        return  ANode->expand();

    if(ANode->is_terminal())
        return ANode;

    //if we are here, there is an issue
    throw std::runtime_error("Error: expansion has note been done");
}
/*---------------------------------------------------------------------------*/
std::pair<double,MCTSAgent::GAME_RESULT> MCTSAgent::simulate(MCTSTree* ANode) {
    auto state =ANode->get_state();

	 //we first check that we are not a winner!!
	 if(state->win())
		  return std::make_pair(m_reward_function->evaluate(state),WIN);

	 bool found_win=false;
	 bool found_lost =false;
    if(!state->win() && !state->is_terminal()) {
		  //TODO checker cette boucle.
        for (int d = 0; d < m_simulation_depth && !found_win && !found_lost; d++) {
        	auto bs = std::dynamic_pointer_cast<gecko::mctsc::BlockingState>(state);

            if (!state->is_terminal()) {
	            auto a = get_random_action(state);

            	if(a== nullptr){
            		//std::cout<<"Error in simulate()"<<std::endl;;
            		//bs->get_blocking()->save_vtk_blocking("simulation_error");
            		//auto score = bs->computeScore();
            	return std::make_pair(m_reward_function->evaluate(state),LOST);
            }
            			//exit(55); //si cela se produit verifier qu'un etat non terminal a bien une list d'action non vide, state.lost() ordre des elements conditionnels

            	auto bs = std::dynamic_pointer_cast<gecko::mctsc::BlockingState>(state);
            //	bs->get_blocking()->save_vtk_blocking("simulation_error");
			//	std::cout<<"We apply action "<<a->get_description()<<std::endl;
                state = a->apply_on(state);
            //	std::cout<<"\t done"<<std::endl;
            	if (state->win())
            		found_win=true;
            	else if (state->lost())
            		found_lost=true;
            }
        }
    }
	 GAME_RESULT result = DRAW;
	 if(state->win())
		  result = WIN;
	 else if (state->lost())
		  result = LOST;
    return std::make_pair(m_reward_function->evaluate(state),result);
}
/*---------------------------------------------------------------------------*/
void MCTSAgent::back_propagate(MCTSTree* ANode, double AReward, MCTSAgent::GAME_RESULT AResult) {
    auto  node = ANode;
    while(node){
		  node->cumulative_reward+=AReward;
		  node->sq_cumulative_reward+=AReward*AReward;
        node->number_visits+=1;
		  if(AResult==WIN)
			   node->number_win+=1;
		  else if(AResult==LOST)
			   node->number_lost+=1;
		 else
			   node->number_draw+=1;
        node = node->get_parent();
    }
}
/*---------------------------------------------------------------------------*/
std::shared_ptr<IAction> MCTSAgent::get_random_action(std::shared_ptr<IState> AState) const {
    /** selects an action among the untried ones */
    auto actions = AState->get_actions_simulation();
	 if (actions.empty())
		  return nullptr;
	 //randomly pick an action
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, actions.size()-1);
	 assert(!actions.empty());
    return actions[distrib(gen)];
}
/*---------------------------------------------------------------------------*/
void MCTSAgent::activate_debug_mode(const std::string &AFileNamePrefix,
                                    const DEBUG_OUTPUT_MODE AOutputMode,
                                    const int AFrequency)
{
    m_debug_activate = true;
    m_debug_file_prefix = AFileNamePrefix;
    m_debug_mode = AOutputMode;
    m_debug_frequency=AFrequency;
}
/*---------------------------------------------------------------------------*/
void MCTSAgent::desactivate_debug_output() {
    m_debug_activate=false;
}
/*---------------------------------------------------------------------------*/
void MCTSAgent::export_tree() {
    static int file_index=0;
    json j;
    std::vector<MCTSTree*> to_do;
    to_do.push_back(m_tree);
    while (!to_do.empty()){
        //get the last node
        auto n = to_do.back();
        to_do.pop_back();

        //export the state of n
        auto state_data = n->get_state()->write(m_debug_file_prefix,file_index,
                              n->get_id(),n->get_depth());

        j["nodes"].push_back(json{{"id",n->get_id()},
                                  {"depth",n->get_depth()},
                                  {"reward",n->cumulative_reward},
                                  {"visits",n->number_visits},
                                  {"data",state_data}});

        if(n->get_parent()!= nullptr){
            //means n is not the root
            j["links"].push_back(json{{"parent",n->get_parent()->get_id()},
                                      {"child",n->get_id()}});

        }
        auto children = n->get_children();
        to_do.insert(to_do.end(),children.begin(),children.end());
    }
    std::ofstream file;
    file.open (m_debug_file_prefix+"_"+std::to_string(file_index)+".json");
    file<<j;
    file.close();
    file_index++;
}

/*---------------------------------------------------------------------------*/
void MCTSAgent::run(std::shared_ptr<IState> ARootState) {
    //check that an existing tree was not used during a previous iteration
    if(!m_tree)
    	m_tree  = new MCTSTree(ARootState);
    //Build the initial tree
    //m_tree  = new MCTSTree(ARootState);
	else {
		for (auto c : m_tree->get_children()) {
			if (c->get_state() == ARootState) {
				promote_child_has_root(c);
			}
		}


	}

    int i=0;
    auto time0 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::ratio<1>> elapsed= std::chrono::steady_clock::now()-time0;

	 while (i<=m_max_iterations && elapsed.count() <= m_max_seconds){

		  std::chrono::time_point<std::chrono::system_clock> start_iter, end_select, end_expand, end_reward, end_back;

		  start_iter = std::chrono::system_clock::now();

        // 1 SELECTION - we explore/exploit the existing tree to find a node to work on
        auto node = select(m_tree);
		  end_select = std::chrono::system_clock::now();
		  std::chrono::duration<double> elapsed_seconds = end_select - start_iter;
	//	 std::cout << "\t node selection    : " << elapsed_seconds.count() << "s\n";

		  // 2. EXPAND by adding a single child (if not terminal or not fully expanded)
        node = expand(node);
		  end_expand = std::chrono::system_clock::now();
		  elapsed_seconds = end_reward - end_select;
	//	 std::cout << "\t node expansion    : " << elapsed_seconds.count() << "s\n";

        // 3. SIMULATE (if not terminal)
        auto [reward, result] = simulate(node);
		  end_reward = std::chrono::system_clock::now();
		  elapsed_seconds = end_reward - end_expand;
	//	std::cout << "\t node simulation   : " << elapsed_seconds.count() << "s\n";
        // 4. BACK PROPAGATION
        back_propagate(node,reward,result);
		  end_back = std::chrono::system_clock::now();
		  elapsed_seconds = end_back - end_reward;
	//	 std::cout << "\t node backprogation: " << elapsed_seconds.count() << "s\n";

        //increase loop counters
        i++;
        elapsed= std::chrono::steady_clock::now()-time0;
        if(m_debug_activate && m_debug_mode==OUT_ITERATION){
            if(i%m_debug_frequency==0)
                export_tree();
        }
	//	 std::cout<<"  -- done"<<std::endl;
    }
    m_nb_iterations=i;
    m_nb_seconds =elapsed.count();
    if (m_debug_activate && (m_debug_mode==OUT_END_ONLY || m_debug_mode==OUT_ITERATION)){
        export_tree();
    }
}
/*---------------------------------------------------------------------------*/
MCTSTree* MCTSAgent::select(MCTSTree* ANode) {
    MCTSTree *node = ANode;
    while (node->is_fully_expanded() && !node->is_terminal()){
        node = m_select_function->select(node);
    }
    //We have reached a terminal node here
    return  node;
}
/*---------------------------------------------------------------------------*/
void MCTSAgent::promote_child_has_root(MCTSTree *new_root) {
	if (!new_root)
		throw std::invalid_argument("Cannot promote null node to root.");

	MCTSTree* parent = new_root->get_parent();
	if (parent) {
		// Facultatif : on pourrait enlever new_root de la liste des enfants de son parent ici
		new_root->set_parent(nullptr);
	}
	m_tree = new_root;
}
/*---------------------------------------------------------------------------*/
