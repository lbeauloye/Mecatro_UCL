#include "motor/path_planning.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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

    if(! isInMap(path_plan, start->i,start->j))
        return NULL;

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

        ClosedList[i][j] = 1;

        for(int k = 0; k<8 ;k++){
            int i_child = i + PossibleChildren[k][0];
            int j_child = j + PossibleChildren[k][1];
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

                        list_append_lowest_f(&OpenList, &Grid[i_child][j_child]);
                        node_open = node_open + 1;
                    }
                }

            }
        }


    }
    free(OpenList);
    return NULL;
}


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
