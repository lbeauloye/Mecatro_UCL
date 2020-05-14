#include "motor/path_planning.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/*! \brief update the path-planning algorithm (recompute Astar)
 * 
 * \param[in,out] cvs controller main structure
 */

//void path_planning_update(CtrlStruct *cvs)
//{
//    PathPlanning *path_plan = cvs->path_plan;
//
//    int i = 100 + (int)floor((cvs->rob_pos->x)/0.01);
//    int j = 150 + (int)floor((cvs->rob_pos->y)/0.01);
//    cvs->path_plan->start = &(cvs->path_plan->Grid[i][j]);
////    cvs->path_plan->start->theta = cvs->rob_pos->theta;
////    if(cvs->strat->prev_state == REOR_STATE || cvs->strat->prev_state == ORIENT_TARGET){
////        cvs->path_plan->start->wLeft = 0.0;
////        cvs->path_plan->start->wRight = 0.0;
////    }
////    else{
////        cvs->path_plan->start->wLeft = round(cvs->inputs->l_wheel_speed/4)*4;
////        cvs->path_plan->start->wRight = round(cvs->inputs->r_wheel_speed/4)*4;
////    }
////    // discrete set of speed
////    double speed[3] = {4.0, 8.0, 12.0};
////    if(cvs->strat->state == STUCK){
////        speed[0] = -speed[0];
////        speed[1] = -speed[1];
////        speed[2] = -speed[2];
////    }
//    List *path = Astarsearch(path_plan, speed);
//    if(path){
//        path_plan->path_found = 1;
//        path_plan->path = path;
//        path_plan->current = Pop(&path_plan->path);
//        path_plan->current = Pop(&path_plan->path);
//        path_plan->start_path = 1;
//        path_plan->last_t = cvs->inputs->t;
//    }
//    else{
//        path_plan->path_found = 0;
//    }
//
//}
//
///*
// * Update the speed command from the current node speeds
// *
// */
//void computeSpeedPath(CtrlStruct *cvs, double& speedL, double& speedR){
//    PathPlanning *path_plan = cvs->path_plan;
//
//    if(path_plan->nbr_nodes_path > 0){
//        if(fabs(cvs->inputs->t-path_plan->last_t) >= path_plan->current->dt || (fabs(100 + (int)floor((cvs->rob_pos->x)/0.01) - cvs->strat->target_list[cvs->strat->target_cnt][0]) <= 2 && fabs(150 + (int)floor((cvs->rob_pos->y)/0.01) - cvs->strat->target_list[cvs->strat->target_cnt][1]) <= 2)){
//
//            // if opponent on the path -> recompute
//            // if last node
//            if(path_plan->nbr_nodes_path == 1){
//                path_plan->last_t = cvs->inputs->t;
//                path_plan->nbr_nodes_path -= 1;
//            }
//            // if too far from position -> recompute
//            else if(fabs(cvs->rob_pos->x - path_plan->current->position_x) > 0.05 || fabs(cvs->rob_pos->y - path_plan->current->position_y) > 0.05)
//            {
//                    printf("Recompute bc far from point \n");
//                    cvs->strat->state = ASTAR;
//
//
//            }
//            else{
//                path_plan->current = Pop(&path_plan->path);
//                path_plan->last_t = cvs->inputs->t;
//                path_plan->nbr_nodes_path -= 1;
//            }
//
//            if(isNewObstacle(cvs->path_plan->path) && cvs->strat->state == DRIVE){
//                printf("recompute because opponent on the way \n");
//                cvs->strat->state = ASTAR;
//            }
//        }
//        speedL = path_plan->current->wLeft;
//        speedR = path_plan->current->wRight;
//    }
//    else{
//        printf("On goal \n");
//        cvs->strat->isOnGoal = 1;
//        speedL = 0.0;
//        speedR = 0.0;
//    }
//}

/*
 * Compute the euclidian distance (heuristic cost)
 *
 */
double h_function_eucl(Node *goal, Node *current){
    return sqrt(pow(goal->position_x-current->position_x, 2) + pow(goal->position_y - current->position_y,2));
}

/*
 * Compute the Manhattan distance (heuristic cost)
 *
 */
double h_function_manh(Node *goal, Node *current){
    return (fabs(goal->position_x-current->position_x) + fabs(goal->position_y - current->position_y));
}

/*
 * Compute the diagonal distance (heuristic cost)
 *
 */
double h_function_diag(Node *goal, Node *current){
    return fmax(fabs(goal->position_x-current->position_x),fabs(goal->position_y - current->position_y));
}

/*
 * Return 1 if the node is in an obstacle
 *
 */
int isObstacle(Node *current){
    return (current->status == 1 || current->status == 2) ;
}

/*
 * Return 1 if the node is in the map
 *
 */
int isInMap(PathPlanning *path_plan, int i, int j){
    
    
    return (i >= 0 && j >= 0) && (i<path_plan->IMAX && j<path_plan->JMAX) ;
}

/*
 * Add a circle representing the opponent on the Grid
 *
 */
void addOpponent(PathPlanning *path_plan, int opp_x, int opp_y){
    int **opp_index = path_plan->opp_index;
	int i_opp = (int) bounded(4 + (int) round(opp_x/10), 0, path_plan->IMAX);
	int j_opp = (int) bounded(8 + (int) round(opp_y/10), 0, path_plan->JMAX);

	int count = 1;
	int index_i, index_j;
	opp_index[0][0] = i_opp;
	opp_index[0][1] = j_opp;
	path_plan->Grid[i_opp][j_opp].status = 2;

	for (int i = -1 + i_opp; i <= 1 + i_opp ; i++){
		for (int j = -1 + j_opp ; j <= 1 + j_opp ; j++){
			if( i != i_opp || j != j_opp){
				index_i = (int) bounded(i, 0, path_plan->IMAX);
				index_j = (int) bounded(j, 0, path_plan->JMAX);
				opp_index[count][0] = index_i;
				opp_index[count][1] = index_j;
				if(path_plan->Grid[index_i][index_j].status == 0){
					path_plan->Grid[index_i][index_j].status = 2;}
				count ++;
			}
		}
	}
}

/*
 * Move the circle corresponding to the opponent on the Grid
 *
 */
void moveOpponent(PathPlanning *path_plan, int opp_x, int opp_y){

	int **opp_index = path_plan->opp_index;
	int i_opp = (int) bounded(4 + (int) round(opp_x/10), 0, path_plan->IMAX);
	int j_opp = (int) bounded(8 + (int) round(opp_y/10), 0, path_plan->JMAX);
	int value;
	int delta_i = i_opp - opp_index[0][0] ;
	int delta_j = j_opp -opp_index[0][1] ;

	if(path_plan->Grid[opp_index[0][0]][opp_index[0][1]].status == 2){
		path_plan->Grid[opp_index[0][0]][opp_index[0][1]].status = 0;}
	if(path_plan->Grid[i_opp][j_opp].status == 0){
		path_plan->Grid[i_opp][j_opp].status = 2;}

	opp_index[0][0] = i_opp;
	opp_index[0][1] = j_opp;

	for(int i = 1; i <9 ;i++){
		// Erase previous position
		if(path_plan->Grid[opp_index[i][0]][opp_index[i][1]].status == 2){
			path_plan->Grid[opp_index[i][0]][opp_index[i][1]].status = 0;}

		value = delta_i + opp_index[i][0];
		opp_index[i][0] =  (int) bounded(value, 0, path_plan->IMAX);
		value = delta_j + opp_index[i][1];
		opp_index[i][1] =  (int) bounded(value, 0, path_plan->JMAX);
		if(path_plan->Grid[opp_index[i][0]][opp_index[i][1]].status == 0){
			path_plan->Grid[opp_index[i][0]][opp_index[i][1]].status = 2;}
	}
}


double bounded(double value, double min_value, double max_value)
{
	double out = 0.0;

	if (value > 0) 	{out = fmin(max_value,value);}
	else			{out = fmax(min_value,value);}

	return out;
}


/*
 * Create a linked list
 *
 */
List *list_create(Node *node)
{
    List *list = (List*) malloc(sizeof(list));
    if (list)
    {
        list->node = node;
        list->next = NULL;
    }
    return list;
}

/*
 * Append a node to the list and organize the list with decreasing f costs
 *
 */
void list_append_lowest_f(List **plist, Node *node)
{
    if(plist){
        List *tmp = NULL;
        List *csl = *plist;
        List *elem = (List*) malloc(sizeof(List));
        elem->node = node;
        while(csl && csl->node->f < node->f)
        {
            tmp = csl;
            csl = csl->next;
        }
        elem->next = csl;
        if(tmp) tmp->next = elem;
        else *plist = elem;
    }
    else
        Push(plist, node);
}

/*
 * Return 1 if an obstacle is now on the computed path
 *
 */
int isNewObstacle(List *list)
{
    while(list)
    {
        if(list->node->status == 2){
            return 1;
        }
        list = list->next;
    }
    return 0;
}

/*
 * Append an element on the list
 *
 */
List *list_append(List *list, Node *node)
{
    List **plist = &list;
    while (*plist)
        plist = &(*plist)->next;
    *plist = list_create(node);
    if (*plist)
        return list;
    else
        return NULL;
}

/*
 * Push an element on the list
 *
 */
void Push(List **list, Node *node){
    List *element = list_create(node);
    element->next = *list;
    *list = element;
}

/*
 * Remove first element out of the list
 *
 */
Node *list_remove_first(List *list)
{
    List *first = list;
    list = list->next;
    printf("next list %d \n", list->node->i);
    return first->node;
}

/*
 * Display all costs of the list
 *
 */
void View(List *p)
{
    while(p)
    {
        printf("%d,\t %d\n",p->node->i,p->node->j);
        p = p->next;
    }
}

/*
 * Pop an item out of a list
 *
 */
Node *Pop(List **p)
{
    Node *node;
    List *tmp;
    if(!*p) return NULL;
    tmp = (*p)->next;
    node = (*p)->node;
    free(*p);
    *p = tmp;
    return node;
}

/*
 * Return 1 if the node found is the goal with a tolerance of 2 cm
 *
 */
int isGoal(int i, int j, Node *goal){
    return (fabs(i-goal->i)<=0 && fabs(j-goal->j)<=0);
}



/*
 *  Retrieve path from the grid
 *
 */
List *findPath(PathPlanning *path_plan, Node **Grid, Node *start, Node *goal){
    int i = goal->i;
    int j = goal->j;
    int i_start = start->i;
    int j_start = start->j;
    double g = 0;
    int i_old;
    int count_node = 0;
    List *path = list_create(goal);
    while(!(i==i_start && j == j_start)){
        i_old = i;
        g = g + Grid[i][j].g;
        i = Grid[i][j].parent_x;
        j = Grid[i_old][j].parent_y;
        
        Push(&path, &Grid[i][j]);
        count_node++;
    }
    path_plan->nbr_nodes_path = count_node;
    // to display cost
//    printf("g : %f \n", goal->g);
    return path;
    
}
/*
 * First version of Astar
 * Compute path without considering non-holonomic robot
 *
 */
 List *Astarsearch(PathPlanning *path_plan)
{

    int nbrx_nodes = path_plan->nbrx_nodes;
    int nbry_nodes = path_plan->nbry_nodes;
    Node *start = path_plan->start;
    Node *goal = path_plan->goal;
    Node **Grid = path_plan->Grid;




//    printf("i : %d, j : %d \n", start->i, start->j);

    if(! isInMap(path_plan, start->i,start->j))
        return NULL;

//    printf("i : %d, j : %d \n", goal->i, goal->j);
    if(! isInMap(path_plan, goal->i,goal->j))
        return NULL;
    List *OpenList = list_create(start);


    int ClosedList[nbrx_nodes][nbry_nodes] ;
    for (int i = 0; i < nbrx_nodes; i++)
    {
        for (int j = 0; j < nbry_nodes; j++){
				ClosedList[i][j] = 0;
				Grid[i][j].f = W_MAX;
				Grid[i][j].g = W_MAX;
				Grid[i][j].h = W_MAX;
				Grid[i][j].parent_x = -1;
				Grid[i][j].parent_y = -1;

        }
    }

    int i_start = start->i;
    int j_start = start->j;

    Grid[i_start][j_start].f = 0;
    Grid[i_start][j_start].g = 0;
    Grid[i_start][j_start].h = 0;
    Grid[i_start][j_start].parent_x = i_start;
    Grid[i_start][j_start].parent_y = j_start;

    int PossibleChildren[8][2] = {{-1, 0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1}, {1,1}};
    int node_open = 1;


    double g, h, f, weight;
    while(node_open != 0){
        Node *current = Pop(&OpenList);
        node_open = node_open - 1 ;
        int i = current->i;
        int j = current->j;

//        printf("i : %d, j : %d \n", i, j);

        ClosedList[i][j] = 1;

        for(int k = 0; k<8 ;k++){
            int i_child = i + PossibleChildren[k][0];
            int j_child = j + PossibleChildren[k][1];
//            printf("i : %d, j : %d \n", i_child, j_child);
            if(isInMap(path_plan,i_child,j_child)){

                if(isGoal(i_child,j_child, goal)){
                    Grid[i_child][j_child].parent_x = i;
                    Grid[i_child][j_child].parent_y = j;
                    if(k > 3){
						weight = sqrt(2.0) * 0.1;
					}
					else{
						weight = 1.0 * 0.1;
					}
                    Grid[i_child][j_child].g = Grid[i][j].g + weight;
                    Grid[i_child][j_child].f = Grid[i_child][j_child].g;
                    free(OpenList);
                    return findPath(path_plan, Grid, start, goal);
//                    break;
                }


                else if((ClosedList[i_child][j_child] != 1) && (! isObstacle(&Grid[i_child][j_child]))){
                    if(k > 3){
                        weight = sqrt(2.0) * 0.1;
                    }
                    else{
                        weight = 1.0 * 0.1;
                    }
                    g = Grid[i][j].g + weight;
                    h = h_function_eucl(goal, &Grid[i_child][j_child]);
                    f  = g + h;
                    if((Grid[i_child][j_child].f == W_MAX) || (f < Grid[i_child][j_child].f)){
                        Grid[i_child][j_child].f = f;
                        Grid[i_child][j_child].g = g;
                        Grid[i_child][j_child].h = h;
                        Grid[i_child][j_child].parent_x = i;
                        Grid[i_child][j_child].parent_y = j;
                        //                        OpenList = list_append(OpenList, &Grid[i_child][j_child]);

                        list_append_lowest_f(&OpenList, &Grid[i_child][j_child]);
                        //                        Push(&OpenList, &Grid[i_child][j_child]);
                        node_open = node_open + 1;
                    }
                }

            }
        }


    }
    free(OpenList);
    return NULL;
}



/*
 * Compute the path with the Astar algorithm
 * speed = array of possible speed e.g {4, 8, 12} [rad/s]
 *
 */
/*
List *Astarsearch(PathPlanning *path_plan, double* speed)
{
    clock_t begin = clock();
    clock_t end = clock();
    int nbrx_nodes = path_plan->nbrx_nodes;
    int nbry_nodes = path_plan->nbry_nodes;
    Node *start = path_plan->start;
    Node *goal = path_plan->goal;
    Node **Grid = path_plan->Grid;
    
    if(! isInMap(path_plan, start->i,start->j))
        return NULL;
    
    if(! isInMap(path_plan, goal->i,goal->j))
        return NULL;
    
    List *OpenList = list_create(start);
    
    // initialize closed list
    int ClosedList[nbrx_nodes][nbry_nodes] ;
    for (int i = 0; i < nbrx_nodes; i++)
    {
        for (int j = 0; j < nbry_nodes; j++)
            ClosedList[i][j] = 0;
    }
    
    int i_start = start->i;
    int j_start = start->j;
    
    // initiliaze start node
    Grid[i_start][j_start].g = 0.0;
    Grid[i_start][j_start].h = h_function_manh(goal, &Grid[i_start][j_start]);
    Grid[i_start][j_start].f = Grid[i_start][j_start].h;
    Grid[i_start][j_start].parent_x = i_start;
    Grid[i_start][j_start].parent_y = j_start;
    
    // posssible children from node given set of speed
    double PossibleChildren[10][2] = {{speed[0], speed[2]},{speed[2],speed[0]},{speed[1],speed[2]}, {speed[2],speed[1]},{speed[1],speed[1]}, {speed[1],speed[1]},{speed[1], speed[1]}, {speed[1], speed[1]},  {speed[0],speed[0]} ,{speed[0],speed[0]}};
    int node_open = 1;
    
    // time step for integration
    double dt[10] = { 2.0, 2.0, 4.0, 4.0, 0.2,  0.2, 0.5, 1.0, 0.2, 0.5};
    // Desired distance to cover
    // if rotation then ds = 0.0 (bc always 90 deg angles)
    double ds[10] = { 0.0, 0.0, 0.0, 0.0, 0.02, 0.04, 0.1, 0.2,  0.02, 0.05};
    int kmax = 8;
    double x, y, cost, theta;
    double g, h, f, weight;
    
    // while there are nodes to visit and do not spend to much time
    while(node_open != 0 && (double)(end - begin) / CLOCKS_PER_SEC <= 1.5){
        
        Node *current = Pop(&OpenList);
        node_open = node_open - 1 ;
        int i = current->i;
        int j = current->j;
        ClosedList[i][j] = 1;
        
        // add precision around goal
        if(h_function_manh(goal, &Grid[i][j])<0.2 ){
            kmax = 10;
        }
        else {
            kmax = 8;
        }
        // all possible children
        for(int k = 0; k<kmax ;k++){
            
            x = Grid[i][j].position_x;
            y = Grid[i][j].position_y;
            theta = Grid[i][j].theta;
            
            //possible children
            int inMap = diffDriveCompute(path_plan, Grid, PossibleChildren[k][0], PossibleChildren[k][1], Grid[i][j].wLeft, Grid[i][j].wRight, dt[k], ds[k], theta, x, y, cost, 0.01, 0.01);
            
            int i_child;
            int j_child;
            if(inMap>0){
                i_child = 100 + (int) round(x/0.01);
                j_child = 150 + (int) round(y/0.01);
            }
            else{
                i_child = -1;
                j_child = -1;
            }
            
            
            if(isInMap(path_plan,i_child,j_child)){
                
                // if at goal
                if(isGoal(i_child,j_child, goal)){
                    Grid[i_child][j_child].parent_x = i;
                    Grid[i_child][j_child].parent_y = j;
                    weight = cost;
                    Grid[i_child][j_child].g = Grid[i][j].g + weight;
                    Grid[i_child][j_child].f = Grid[i_child][j_child].g;
                    Grid[i_child][j_child].theta = theta;
                    Grid[i_child][j_child].wLeft = PossibleChildren[k][0];
                    Grid[i_child][j_child].wRight = PossibleChildren[k][1];
                    free(OpenList);
                    return findPath(path_plan, Grid, start, &Grid[i_child][j_child]);
                }
                
                // if not yet chosen and reachable
                else if((ClosedList[i_child][j_child] != 1) && (! isObstacle(&Grid[i_child][j_child]))){
                    weight = cost;
                    g = Grid[i][j].g + weight;
                    h = h_function_manh(goal, &Grid[i_child][j_child]);
                    f  = g + h;
                    
                    // if not yet explored or  with a better cost
                    if((Grid[i_child][j_child].f == W_MAX) || (f < Grid[i_child][j_child].f)){
                        Grid[i_child][j_child].f = f;
                        Grid[i_child][j_child].g = g;
                        Grid[i_child][j_child].h = h;
                        Grid[i_child][j_child].parent_x = i;
                        Grid[i_child][j_child].parent_y = j;
                        Grid[i_child][j_child].theta = theta;
                        Grid[i_child][j_child].wLeft = PossibleChildren[k][0];
                        Grid[i_child][j_child].wRight = PossibleChildren[k][1];
                        Grid[i_child][j_child].dt = dt[k];
                        list_append_lowest_f(&OpenList, &Grid[i_child][j_child]);
                        node_open = node_open + 1;
                    }
                }
                
            }
        }
        
     end = clock();
    }
    free(OpenList);
    return NULL;
}
*/
/*
 * Version of diffDriveCompute that does not take the acceleration into account andn with a fixed dt
 *
 */

/*int diffDriveCompute(PathPlanning *path_plan, Node **Grid, double Left, double Right, double dt, double& theta, double& x, double& y, double& cost, double dx1, double dy1){
    
    //    printf("x %f\n", x);
    double sample = 10;
    
    double dvx = 0.0;
    double dvy = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    double R = 0.03;
    double b = 0.225;
    double dtheta = 0.0;
    double w = (Right-Left)*R/b;
    cost = 0.0;
    
    for(int i = 0; i<sample; i++){
        dvx = R * 0.5 * (Left+Right) * cos(theta);
        dvy = R * 0.5 * (Left+Right) * sin(theta);
        dx = dvx * dt/sample;
        dy = dvy * dt/sample;
        dtheta = w * dt/sample;
        x += dx;
        y += dy;
        theta += dtheta;
        cost += sqrt(dx*dx + dy*dy);
        
        if(isInMap(path_plan, 100 + (int)floor(x/dx1), 150 + (int)floor(y/dy1)) == 0){
            return -1;
        }
        else if(isObstacle(&Grid[100 + (int)floor(x/dx1)][150 + (int)floor(y/dy1)])){
            return -1;
        }
        
    }
    return 1;
}*/


/* Compute the estimated arrival point in the Grid with 2 wheel speeds and a predefined distance
 * param In : Left : possible left wheel speed, Right : possible right wheel speed, prevL : current left wheel speed, prevR : current right wheel speed,
 * ds : predefined distance to reach, dx1 : nbr of cm between nodes along x axis, dy1 : nbr of cm between nodes along y axis
 * param Out : dt : time to reach the distance (dt in input is shortened), cost : the cost of the incremental trajectory
 * param In, Out : modified values of theta, x and y corresponding to the arrival point
 *
 */
//int diffDriveCompute(PathPlanning *path_plan, Node **Grid, double Left, double Right, double prevL, double prevR, double& dt, double ds, double& theta, double& x, double& y, double& cost, double dx1, double dy1){
//
//    // determine the time interval
//    double sample = 100;
//
//    double dvx = 0.0;
//    double dvy = 0.0;
//    double dx = 0.0;
//    double dy = 0.0;
//    // Experimental acceleration
//    double slope = 12.0/0.4;
//    // acceleration time left
//    double dt_acc_y = (Left-prevL)/slope;
//    //acceleration time right
//    double dt_acc_x = (Right-prevR)/slope;
//    double R = 0.03;
//    double b = 0.225;
//    double dtheta = 0.0;
//    double v_left, v_right;
//    double w = (Right-Left)*R/b;
//    double theta_0 = theta;
//    double x_0 = x;
//    double y_0 = y;
//    int rot = (w!=0);
//    cost = 0.0;
//    int i;
//    for(i = 0; i<sample; i++){
//        // take acceleration into account
//        if(dt/sample*(i) < dt_acc_y){
//            v_left = prevL + (Left-prevL)/dt_acc_y * dt/sample * i;
//        }
//        else{
//            v_left = Left;
//        }
//        if(dt/sample*(i) < dt_acc_x){
//            v_right = prevR + (Right-prevR)/dt_acc_x * dt/sample * i;
//        }
//        else{
//            v_right = Right;
//        }
//
//        dvx = R * 0.5 * (v_left+v_right) * cos(theta);
//        dvy = R * 0.5 * (v_left+v_right) * sin(theta);
//        dx = dvx * dt/sample;
//        dy = dvy * dt/sample;
//        w = (v_right-v_left)*R/b;
//
//        dtheta = w * dt/sample;
//
//        // new x, y, theta
//        x += dx;
//        y += dy;
//        theta += dtheta;
//        if(rot){
//            // if rotation stop when angle = +- M_PI/2
//            if(fabs(theta-theta_0)>= M_PI/2){
//                dt = (i+1)*dt/sample;
//                break;
//            }
//        }
//        // if straight line stop when distance reached
//        else if(sqrt((x-x_0)*(x-x_0) + (y-y_0)*(y-y_0))>= ds){
//            dt = (i+1)*dt/sample;
//            break;
//        }
//        cost += sqrt(dx*dx + dy*dy);
//
//        // stop the computation if path go out of map or on an obsatcle
//        if(isInMap(path_plan, 100 + (int)floor(x/dx1), 150 + (int)floor(y/dy1)) == 0){
//            return -1;
//        }
//        else if(isObstacle(&Grid[100 + (int)floor(x/dx1)][150 + (int)floor(y/dy1)])){
//            return -1;
//        }
//
//    }
//    return 1;
//}

/* Map the domain into a Grid
 * Puts default values to all nodes
 */

void mapping(PathPlanning *path_plan)
{
    double x_length = 0.8;
    double y_length = 1.7;
    int nbrx_nodes = path_plan->nbrx_nodes;
    int nbry_nodes = path_plan->nbry_nodes;
    Node **Grid = path_plan->Grid;
    
    double dx = x_length/nbrx_nodes;
    double dy = y_length/nbry_nodes;
    int i;
    int j;
    for(i=0; i < nbrx_nodes; i++){
        for(j=0; j < nbry_nodes; j++){
            Grid[i][j].position_x = -0.4 + i*dx;
            Grid[i][j].position_y = -0.8 + j*dy;
            Grid[i][j].i = i;
            Grid[i][j].j = j;
            Grid[i][j].f = W_MAX;
            Grid[i][j].g = W_MAX;
            Grid[i][j].h = W_MAX;
            Grid[i][j].theta = 0.0;
            Grid[i][j].dt = 1.0;
            Grid[i][j].parent_x = -1;
            Grid[i][j].parent_y = -1;
            Grid[i][j].status = 0;
        }
        path_plan->JMAX = j;
    }
    path_plan->IMAX = i;
    
}
