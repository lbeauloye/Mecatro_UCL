/*! 
 * \author gr5 based on Nicolas Van der Noot
 * \file path_planning_gr5.h
 * \brief path-planning algorithm
 */

#ifndef _PATH_PLANNING_H_
#define _PATH_PLANNING_H_


# define W_MAX 1000

/// path-planning main structure
typedef struct s_node Node;
struct s_node{
    double position_x;
    double position_y;
    int parent_x;
    int parent_y;
    int i;///< index along x axis
    int j;///< index along y axis
    double f;/// total cost
    double g;///< real cost
    double h;///< Heurisitic cost
    double theta;///< Angle at that node
    double wLeft;///< Left wheel speed at that node
    double wRight;///< Right wheel speed at that node
    double dt;///< Time interval to interpolate the speed
    int status; ///< 1 if obstacle, 2 if opponent, 0 otherwise
    Node *parent;///< Parent node
};



typedef struct s_List List;
struct s_List
{
    List *next; /* pointer to the rest of the list */
    Node *node; /* pointer to a node */
};

typedef struct PathPlanning
{
    int path_found; ///< 1 if astar found a path
    int IMAX; ///< limit of index in the map
    int JMAX;
    int nbrx_nodes;///< nbr of nodes along the x axis
    int nbry_nodes;///< nbr of nodes along the y axis
    int nbr_nodes_path;///< nbr of nodes in the path
    int start_path;///< if the node is the first one
    int recompute;
    double last_t;
    double *position_x_node;///< not used
    double *position_y_node;///< not used
    double *opp_pos;
    int target_list[3][2];
    int target_cnt;
    int **opp_index;
    int move_opp;
    Node *start;///< Start node
    Node *goal;///< goal node
    Node *current;
    Node **Grid;///< Grid with all the nodes
    List *path;///< found path
    
}PathPlanning;


//void path_planning_update(CtrlStruct *cvs);

double h_function_eucl(Node *goal, Node *current);

double h_function_manh(Node *goal, Node *current);

double h_function_diag(Node *goal, Node *current);

int isObstacle(Node *current);

int isInMap(PathPlanning *path_plan, int i, int j);

List *list_create(Node *node);

List *list_append(List *list, Node *node);

Node *list_remove_first(List *list);

Node *Pop(List **p);

void list_append_lowest_f(List **list, Node *node);

void View(List *p);

void Push(List **list, Node *node);

int isGoal(int i, int j, Node *goal);

int isNewObstacle(List *list);

void addOpponent(PathPlanning *path_plan, int opp_x, int opp_y);

void moveOpponent(PathPlanning *path_plan, int opp_x, int opp_y);

double bounded(double value, double min_value, double max_value);

//int diffDriveCompute(PathPlanning *path_plan, Node **Grid, double Left, double Right,  double prevL, double prevR, double& dt, double ds, double& theta, double& x, double& y, double& cost,double dx1, double dy1);

//void computeSpeedPath(CtrlStruct *cvs, double& speedL, double& speedR);

List *findPath(PathPlanning *path_plan, Node **Grid, Node *start, Node *goal);

List *Astarsearch(PathPlanning *path_plan);

void mapping(PathPlanning *path_plan);


#endif
