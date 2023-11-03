/***
Copyright (c) 2023 Guiraud Maël & Strozecki Yann
All rights reserved.
*///

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include<sys/time.h>

#define PERIODE 100
#define NB_ROUTES 100
#define TAILLE_ROUTES 100
#define NB_SIMUL 1000
#define EXACT_RESOLUTION 0 //Set to 1 to run the FPT algorithm
#define FIG18 1 // set to 1 if you want to recreate the file time.data for figure 18
#define DEBUG 0

double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}
typedef struct{
	int *aller;
	int *retour;
	int *decalages;
	int periode;
	int nb_routes;
} entree;

//Pour le new bruteforce
typedef struct{
	int *next;
	int margin;
}stack;



int collision(int message_size, int period, int slot, int *messages, int level){ //slot is the number 
	//of the slot in the period in which the message is not stacked against another one
	if(slot > period - message_size) return 1; //special case of the first message
	int i;
	for(i = 0; i < level 
		&& (slot <= messages[i] - message_size 
		|| slot >= messages[i] + message_size ); i++){}
	return (i != level);
}

int research_interval(int slot, int *messages, int level, int *min, int *max){
	//return the index of the element before the one which is inserted
	int previous = 0;
	for(int i = 1; i < level; i++){
		if(slot > messages[i] && messages[i] > *min) {
			*min = messages[i];
			previous = i;
		}
		if(messages[i] > slot && messages[i] < *max) *max = messages[i];
	}
	return previous;
}


void update_solution(int *id, int* start_slot, int *return_slot,stack *fw, stack *bw,
 int added_route, int previous_route, int slot, int route_number, int period, int message_size, 
 int min, int max, int level, int previous_index){
	//update the solution
	id[level] = added_route;
	start_slot[level] = start_slot[previous_route] + message_size;
	return_slot[level] = slot;
	//update the two stacks
	for(int i = 0; i < level; i++){
		fw[level].next[i] = (fw[level-1].next[i] <= added_route) ? added_route + 1 :  fw[level-1].next[i];
		bw[level].next[i] = (bw[level-1].next[i] <= added_route) ? added_route + 1 :  bw[level-1].next[i];
	}
	//one cannot use previous route anymore
	fw[level].next[previous_route] = route_number;

	//compute whether we can put something before or after route_added
	bw[level].next[level] = (return_slot[level] + 2*message_size > max ) ? route_number : 1;
	if(min + 2*message_size > slot) bw[level].next[previous_index] = route_number;

	//find the message after the one we are inserting in the fw_period
	max = period;
	for(int i = 1; i < level; i++){
		if(start_slot[i] > start_slot[level] && start_slot[i] < max) max = start_slot[i];
	}
	// compute if there is some room after the message we have just inserted
	fw[level].next[level] = (max - start_slot[level] < 2*message_size) ? route_number : 1;
}

int recursive_search(int *id, int*start_slot, int *return_slot, stack *fw, stack* bw, int *unused_route, int level,
 int* return_time, int route_number, int message_size, int period){ //renvoie 1 si une solution a été trouvée, 0 sinon
	//level is the current depth in the search tree, it corresponds to solutions with level+1 elements 
	//print_solution(id, start_slot, return_slot,level-1, fw, bw);
	if(level == route_number ) return 1; //we have placed all messages
	//place a message next to another one in the forward windows
	int slot, min, max, previous_index;
	for(int i = 0; i < level; i++){//go through the  messages already placed 
		for(int j = fw[level-1].next[i]; j < route_number; j++){//add a message next to the message number i in the forward windows
			if(unused_route[j]){
				slot = start_slot[i] + message_size + return_time[j];
				if (slot > period) slot -= period; //put retour in the interval modulo period
				//test whether there is a collision
				if(collision(message_size, period, slot, return_slot, level)) continue;
				//compute the min and max
				min = 0; max = period;
				previous_index = research_interval(slot,return_slot,level,&min,&max);
				//update the bw_margin and skip this partial solution if it is smaller than 0	
				bw[level].margin = bw[level -1].margin + (max - slot)/message_size 
				+ (slot - min)/message_size - (max - min)/message_size;
				if(bw[level].margin < 0) continue;
				fw[level].margin = fw[level-1].margin; // this margin does not change
				update_solution(id, start_slot, return_slot,fw, bw, j, i, slot,route_number, period, message_size, min, max, level, previous_index); //update all informations in the stacks and solutions
				unused_route[j] = 0;
				if(recursive_search(id,start_slot,return_slot,fw,bw,unused_route,level+1,return_time,route_number, message_size,period)) return 1; // we have found a solution, exit
					//we have not found a solution go one level back in the tree
				unused_route[j] = 1;
			}
		}
		//place a message next to another one in the backward windows
		//same code but the  role of fw and bw variables are exchanged
		for(int j = bw[level-1].next[i]; j < route_number; j++){//add a message next to the message number i in the forward windows
			if(unused_route[j]){
				slot = return_slot[i] + message_size - return_time[j];
				if (slot < 0) slot += period; //put retour in the interval modulo period
				//test whether there is a collision
				if(collision(message_size, period, slot, start_slot,level)) continue;
				//compute the min and max
				min = 0; max = period;
				previous_index = research_interval(slot,start_slot,level,&min,&max);
				//update the bw_margin and skip this partial solution if it is smaller than 0	
				fw[level].margin = fw[level -1].margin + (max - slot)/message_size 
				+ (slot - min)/message_size - (max - min)/message_size;
				if(fw[level].margin < 0) continue;
				bw[level].margin = bw[level-1].margin; // this margin does not change
				update_solution(id, return_slot, start_slot,bw, fw, j, i, slot,route_number, period, message_size, min, max, level, previous_index); //update all informations in the stacks and solutions
				unused_route[j] = 0;
				if(recursive_search(id,start_slot,return_slot,fw,bw,unused_route,level+1,return_time,route_number, message_size,period)) return 1; // we have found a solution, exit
					//we have not found a solution go one level back in the tree
				unused_route[j] = 1;
			}
		}
	}
	return 0;
}


int search(int * return_time,int route_number, int period,int message_size){
	

	int shift = return_time[0];
	for (int i = 0; i < route_number; i++){//we shift the values so that the first route has return time 0
		return_time[i] -= shift;
		if (return_time[i] < 0) return_time[i] += period;
	
	}
	
    /* Memory allocation */		
	stack *fw = malloc(sizeof(stack)*route_number);
	stack *bw = malloc(sizeof(stack)*route_number);
	int *id = malloc(route_number*sizeof(int));
  	int *start_slot = malloc(route_number*sizeof(int));
  	int *return_slot = malloc(route_number*sizeof(int));
  	int *unused_route = malloc(route_number*sizeof(int));

  	for(int i = 0; i < route_number; i++){ 
  		fw[i].next = malloc(sizeof(int)*(i+1));
  		bw[i].next = malloc(sizeof(int)*(i+1));
  		unused_route[i] = 1;
  	}
  	/* Initialization, the first route is fixed */
  	
  	fw[0].next[0] = 1;
  	bw[0].next[0] = 1;
  	fw[0].margin = period /message_size - route_number;
  	bw[0].margin = fw[0].margin;
  	id[0] = 0;
  	start_slot[0] = 0;
  	return_slot[0] = 0;
  	
  	/* Call the recursive part with the proper algorithm */
  	int return_value = recursive_search(id, start_slot, return_slot, fw, bw, unused_route, 1, return_time, route_number, message_size, period);
  		//printf("Solution trouvée \n");
  		//print_solution(id, start_slot, return_slot,route_number-1, fw, bw);
  	/* Free the memory */

  	free(id);
  	free(start_slot);
  	free(return_slot);
  	free(unused_route);
  	for(int i = 0; i < route_number ; i++) {
  		free(fw[i].next);
  		free(bw[i].next);
  	}
  	free(fw);
  	free(bw);
  	return return_value?route_number:0;
}
void init_etoile(entree e, int taille_max)
{
	for(int i=0;i<e.nb_routes;i++)
	{
		e.decalages[i] = rand()%taille_max;
	}
	memset(e.aller,0,sizeof(int)*e.periode);
	memset(e.retour,0,sizeof(int)*e.periode);
}

void print_solution(int* aller, int *retour, int taille){
	printf("\nAller :\n");
	for(int i = 0; i < taille; i++){
		printf("%d ",aller[i]);
	}
	printf("\n Retour : \n");
	for(int i = 0; i < taille; i++){
		printf("%d ",retour[i]);
	}
	printf("\n");
}

int choix_uniforme(entree e, int route_courante){
	int cmpt = 0;
	for(int i=0; i<e.periode; i++){
		cmpt += (!e.aller[i] && !e.retour[ (i+e.decalages[route_courante])%e.periode]);
	}
	if (cmpt == 0) return -1;
	int alea = rand()%cmpt;
	cmpt = -1;
	for(int i=0; ; i++){
		cmpt += (!e.aller[i] && !e.retour[ (i+e.decalages[route_courante])%e.periode]);
		if(cmpt == alea) return i;		
	}
}

int first_fit(entree e, int route_courante){
	int i;
	for(i=0; i<e.periode && (e.aller[i] || e.retour[(i+e.decalages[route_courante])%e.periode]); i++){}
	return (i == e.periode) ? -1 : i;
}


int potential(entree e, int route_courante){

	int max_profit = -1;
	int max_pos = -1;
	for(int i=0; i<e.periode; i++){
		if(!e.aller[i] && !e.retour[(i+e.decalages[route_courante])%e.periode]){
			int temp =  0;
			for(int j = route_courante + 1; j<e.nb_routes; j++){
				temp+=  e.retour[(i + e.decalages[j])%e.periode] + 
				e.aller[(i + e.decalages[route_courante] + e.periode -e.decalages[j])%e.periode];
			}
			if(temp > max_profit){
				max_profit = temp;
				max_pos = i;
			}
		}
	}
	return max_pos;
}


//Renvoie le nombre de route que l'algo a réussi à scheduler
//en argument la règle de choix avec en argument aller, retour, decalages, periode, nombre de routes, route courante
int greedy(entree e, int (choix)(entree,int))
{
	
	int nb_routes_placees = 0;

	for(int i=0;i<e.nb_routes;i++)
	{
		int position = choix(e,i);
		if(position != -1)
		{
			e.aller[position]=1;
			e.retour[ (position+e.decalages[i])%e.periode] = 1;
			nb_routes_placees++;
		}
		
	}
	return nb_routes_placees;
}


int greedy_uniform(entree e){
	return greedy(e, choix_uniforme);
}


int greedy_first_fit(entree e){
	return greedy(e, first_fit);
}

int greedy_potential(entree e){
	return greedy(e,potential);
}

//Pour évaluer le profit correctement, il faut gérer les répétitons,
//mais comme ce bout de profit est un invariant qui ne dépend pas de quand on 
//place la route, ne sert à rien
/*int frequence(int val, int *tab, int taille){ //calcule la fréquence du premier élément dans le tableau
	int cmpt = -1;
	for(int i = 0; i < taille; i++){
		cmpt += (val == tab[i]);
	}
	return cmpt;
}
*/

int greedy_advanced(entree e)
{
	int *profit_route = calloc(e.nb_routes,sizeof(int));
	int *profit_aller = calloc(e.periode,sizeof(int));
	int *profit_retour = calloc(e.periode,sizeof(int));
	
	int nb_routes_placees = 0;
	if(DEBUG){
		printf("Instance : \n");
		for(int i = 0; i<e.nb_routes; i++){
			printf("%d ",e.decalages[i]);
		}
		printf("\n");	
	}
	for(int i=0;i<e.nb_routes;i++)
	{
		int delta_profit = -e.nb_routes; //cannot be smallest than that
		//it evalutes the difference in profit with the previous step, should be
		//positive most of the time
		int max_pos = -1;
		int max_route = -1;
		for(int route = i; route < e.nb_routes; route++){
			for(int pos = 0; pos< e.periode; pos++){
				if(!e.aller[pos] && !e.retour[(pos + e.decalages[route])%e.periode]){
					//if the route can be placed at this position
					int temp_prof = profit_aller[pos] + 
					profit_retour[(pos + e.decalages[route])%e.periode] -
					profit_route[route];// + frequence(e.decalages[route],e.decalages+i, e.nb_routes -i);
					if(temp_prof > delta_profit){
						max_pos = pos;
						max_route = route;
						delta_profit = temp_prof;
					}
				}
			}
		}
		if(DEBUG){
			printf("\nDelta profit: %d, max pos: %d,max route: %d, decalage: %d\n",delta_profit, max_pos,max_route,e.decalages[max_route]);
		}
		if(max_pos != -1)
		{
			int max_pos_retour = (max_pos+e.decalages[max_route])%e.periode; 
			for(int pos=0; pos < e.periode;pos++){//remove one for the positions
				//not profitable for route_max anymore because it is fixed
				profit_retour[(e.decalages[max_route] + pos)%e.periode] -= e.aller[pos];
				profit_aller[pos] -= e.retour[(e.decalages[max_route] + pos)%e.periode];
			}
			e.aller[max_pos]=1;
			e.retour[max_pos_retour] = 1;
			nb_routes_placees++;
			//the max_route is put in position i to not be used again
			int temp = e.decalages[i];
			e.decalages[i] = e.decalages[max_route];
			e.decalages[max_route] = temp;
			temp = profit_route[i];
			profit_route[i] = profit_route[max_route];
			profit_route[max_route] = temp;

			//add one for each active position because we use max_position now
			//update the corresponding routes
			for(int route=i+1; route < e.nb_routes; route++){
				profit_retour[(e.decalages[route] + max_pos)%e.periode]++;
				profit_route[route] += e.retour[(e.decalages[route] + max_pos)%e.periode];
				profit_aller[(-e.decalages[route] + max_pos_retour + e.periode)%e.periode]++ ;
				profit_route[route] += e.aller[(-e.decalages[route] + max_pos_retour + e.periode)%e.periode];
			}
			
			//affichage des profits
				if(DEBUG){
				printf("Profit aller: \n");
				for(int i = 0; i < e.periode; i++){
					printf("%d ",profit_aller[i]);
				}
				printf("\nProfit retour: \n");
				for(int i = 0; i < e.periode; i++){
					printf("%d ",profit_retour[i]);
				}
				printf("\nProfit route: \n");
				for(int i = 0; i < e.nb_routes; i++){
					printf("%d ",profit_route[i]);
				}
			}
		}	
		else{
			return nb_routes_placees;
		}
	}
	if(DEBUG) print_solution(e.aller,e.retour,e.periode);
	return nb_routes_placees;
}

int schedule(entree e, int*offsets, int route){//schedule the route at the first possible position
	int pos = first_fit(e,route);
	if(pos != -1){
		offsets[route] = pos;
		e.aller[pos]=1;
		e.retour[ (pos+e.decalages[route])%e.periode] = 1;
		return 1;
	}
	return 0;
}

void unschedule(entree e, int *offsets, int route){//remove the route from the partial solution

	if(offsets[route] == -1) {
		printf("unscheduled une route non scheduled : %d\n",route);
		printf("Offsets");
		for(int i=0; i < e.nb_routes; i++) printf("%d ",offsets[i]);
		printf("\nDecalages");
		for(int i=0; i < e.nb_routes; i++) printf("%d ",e.decalages[i]);
		print_solution(e.aller,e.retour,e.periode);

	}
	if(route <0 || route >= e.nb_routes) printf("un probleme \n");
	e.aller[offsets[route]] = 0;
	e.retour[(offsets[route]+e.decalages[route])%e.periode] = 0;
	offsets[route] = -1;
}

void test_sol(entree e, int *offsets){//test that offsets correspond to traces
	int * aller = calloc(e.periode,sizeof(int));
	int * retour = calloc(e.periode,sizeof(int));
	//construction à partir des offsets
	for(int i = 0; i < e.nb_routes; i++){
		if(offsets[i] == -1 ) 
		{
			printf("offset à -1\n");
		}
		else{
			aller[offsets[i]]++;
			retour[(offsets[i] + e.decalages[i])%e.periode]++;
		}
	}
	for(int i = 0; i < e.periode; i++){
		if(e.aller[i] != aller[i] || e.retour[i]!= retour[i]) printf("Incohérence offset/trace \n");
	}
}

int all_fit(entree e, int *offsets){
	int res = 0;
	for(int i = 0; i < e.nb_routes; i++){
		if(offsets[i] == -1){
			res += schedule(e,offsets,i);
		}
	}
	return res; //return the number of scheduled solutions
}

int eval_pos(entree e, int pos){
	int val = 0;	
	for(int i = 0; i < e.nb_routes; i++){
		val += e.retour[(pos + e.decalages[i])%e.periode];
	}
	return val;
}

int first_unscheduled(int size, int *offsets){//select the first unscheduled route
	int route;
	for(route = 0; route < size && offsets[route]>=0; route++){}
		return route;
}

int route_from_first_period(entree e, int *offsets, int pos){
	int i;
	for( i = 0; i < e.nb_routes && offsets[i] != pos; i++){}
	return i;
}

int route_from_second_period(entree e, int *offsets, int pos){
	int i;
	for(i = 0; i < e.nb_routes &&
	 (offsets[i] == -1 ||((offsets[i] + e.decalages[i]) % e.periode) != pos); i++){}
	return i;
}

int improve_potential(entree e, int *offsets)
{
	int route = first_unscheduled(e.nb_routes,offsets);
	
	//look for the first permutation which increases the potential
	for(int i = 0; i < e.periode; i++){
		int pos_retour = (i + e.decalages[route]) % e.periode;
		if(!e.aller[i] && e.retour[pos_retour]){
			int route_to_remove = route_from_second_period(e,offsets, pos_retour);	
			if(eval_pos(e,route) > eval_pos(e,route_to_remove)){
				unschedule(e,offsets,route_to_remove);
				e.aller[i] = 1;
				e.retour[pos_retour] = 1;
				offsets[route] = i;
				return 1;//success, the potential has been improved
			}		
		}
	}
	return 0;
}

int exchange(entree e, int *offsets, int route){//try to find an offset for route
	//such that any route moved because of that can be rescheduled
	int route1,route2;
	for(int i = 0; i < e.periode; i++){//first case, empty position in the second period
		int pos_retour = (i + e.decalages[route])%e.periode;
		if(!e.retour[pos_retour]){
			route1 = route_from_first_period(e, offsets, i);
			//remove route1
			unschedule(e,offsets,route1);
			e.aller[i] = 1;
			e.retour[pos_retour]=1;
			offsets[route] = i;
			if(schedule(e,offsets,route1)){
				return 1;//success the route has been rescheduled
			}
			else{//route1 cannot be moved, we rollback the changes
				offsets[route1]= i;
				e.retour[(i + e.decalages[route1])%e.periode] = 1;
				e.retour[pos_retour] = 0;
				offsets[route] = -1;
			}
		}
	}
	for(int i = 0; i < e.periode; i++){//second case, empty position in the first period
		int pos_retour = (i + e.decalages[route])%e.periode;
		if(!e.aller[i]){
			route1 = route_from_second_period(e, offsets, pos_retour);
			//remove route1
			int temp = offsets[route1];
			unschedule(e,offsets,route1);
			e.aller[i] = 1;
			e.retour[pos_retour]=1;
			offsets[route] = i;
			if(schedule(e,offsets,route1)){
				return 1;//success the route has been rescheduled
			}
			else{//route1 cannot be moved, we rollback the changes
				offsets[route1]= temp;
				offsets[route] = -1;
				e.aller[temp] = 1;
				e.aller[i]=0;
			}
		}
	}
	for(int i = 0; i < e.periode; i++){//third case, both routes should move
		int pos_retour = (i + e.decalages[route])%e.periode;
		if(e.aller[i] && e.retour[pos_retour]){
			route1 = route_from_first_period(e, offsets, i);
			route2 = route_from_second_period(e, offsets, pos_retour);
			//remove route1 and 2
			int temp = offsets[route2];
			if(route1 == route2) continue;
			unschedule(e,offsets,route1);
			unschedule(e,offsets,route2);//unschedule ecrit à -1
			e.aller[i] = 1;
			e.retour[pos_retour]=1;
			offsets[route] = i;
			if(schedule(e,offsets,route1)){
				if(schedule(e,offsets,route2)){
					return 1;//the two routes have been rescheduled
				}
				unschedule(e,offsets,route1);
			}
			if(schedule(e,offsets,route2)){//try to schedule in the order route2 then route1
				if(schedule(e,offsets,route1)){
					return 1;//the two routes have been rescheduled
				}
				unschedule(e,offsets,route2);
			}
			//remove route and reschedule both route1 and route2 at their original position
			offsets[route] = -1;
			offsets[route1] = i;
			offsets[route2] = temp;
			e.aller[temp] = 1;
			e.retour[(i + e.decalages[route1])%e.periode] = 1;
		}
	}
	return 0;
}

int swap(entree e){
	int *offsets = malloc(sizeof(int)*e.nb_routes);
	for(int i = 0; i < e.nb_routes; i++){
		offsets[i] = -1;
	}
	int scheduled_message = 0;
	while(1){
		scheduled_message += all_fit(e,offsets);
		if(scheduled_message == e.nb_routes) return e.nb_routes;
		int progress = 1;
		while(progress){ //alternate first fit and improve potential while no of the two methods change the assignment
			while(improve_potential(e,offsets)){}//improve the potential as much as possible
			progress = all_fit(e,offsets); //progress is the number of message scheduled by all fit
			scheduled_message+= progress;
			if(scheduled_message == e.nb_routes) return e.nb_routes;
		}
		

		//if(all_fit(e,offsets) == e.nb) {test_sol(e,offsets);return e.nb_routes;}
		//while(improve_potential(e,offsets)){}//improve the potential as much as possible
		//if(all_fit(e,offsets)) {test_sol(e,offsets);return e.nb_routes;}//try to add routes again now the potential has been improved
										//this can only improve the potential again
		
		if (scheduled_message < e.nb_routes && !exchange(e, offsets, first_unscheduled(e.nb_routes,offsets))){
			break;
		}
		else{
			scheduled_message++;
		}
		//try to schedule a route, if there is one left by moving other routes, if it fails, the algorithm finishes
	}
	return scheduled_message;
} 
//trie le tableau decalage en fonction des releases
void tri_bulles(int* releases,int* decalage,int taille)
{
	int sorted;
	int tmp;
	int tmp_ordre;

	int tabcpy[taille];
	for(int i=0;i<taille;i++)tabcpy[i]=releases[i];

	for(int i=taille-1;i>=1;i--)
	{
		sorted = 1;
		for(int j = 0;j<=i-1;j++)
		{

			if(tabcpy[j+1]<tabcpy[j])
			{
				tmp_ordre = decalage[j+1];
				decalage[j+1]=decalage[j];
				decalage[j]=tmp_ordre;
				tmp = tabcpy[j+1];
				tabcpy[j+1]= tabcpy[j];
				tabcpy[j]= tmp;
				sorted = 0;
			}
		}
		if(sorted){return;}
	}

}

int shortestlongest(entree e)
{
	int period = e.periode ;
	int nb_routes = e.nb_routes ;


	
	int budget = period-nb_routes;

	int release[nb_routes];
	for(int i=0;i<nb_routes;i++)
	{
		//printf("%d %d \n",release[i],e.decalages[i]);
		release[i]= e.decalages[i];
	}

	tri_bulles(e.decalages,release,nb_routes);
	int nb_routes_ok=0;
	int offset = 0;
	for(int i=0;i<nb_routes;i++)
	{
		if(i>0)
		{

			budget -= release[i]-release[i-1];
			//printf("%d %d %d %d %d \n",i,nb_routes,budget,release[i],release[i-1]);
			if(budget < 0)
			{
				break;
			}
		}
		offset = i;
		e.aller[nb_routes_ok]=offset;
		e.retour[nb_routes_ok]=(offset+release[i])%period;
		nb_routes_ok++;
		

	
	}	
	return nb_routes_ok;
}

int recsearch(entree e){
	return search(e.decalages,e.nb_routes,e.periode,1); 
}

double prob_set(int n, int m){
	double res = 1;
	for(int i = 0; i < m-n; i++){
		res*= ((double)(i + 2*n - m + 1 ))/((double)(n + i + 1));
	}
	//printf("%f ",res);
	return res;
}

double prob_theo(int n, int m){ //question, est-ce que faire le produit simplifie les termes ?
	double res = 1;  
	for(int i = m/2; i < n; i++){
		res *= (1-prob_set(i,m));
	}
	return res;
}


float statistique(int periode, int nb_routes, int taille_max, int nb_simul, int seed, int (algo)(entree),char* name){

	srand(seed);
	entree e;
	e.periode = periode;
	e.nb_routes = nb_routes;
	e.aller = malloc(sizeof(int)*e.periode);
	e.retour = malloc(sizeof(int)*e.periode);
	e.decalages = malloc(sizeof(int)*e.nb_routes);
	int success = 0;
	for(int i=0;i<nb_simul;i++){
		init_etoile(e,taille_max);
		success += (algo(e) == nb_routes);
		fprintf(stdout,"\r%d/%d",i+1,NB_SIMUL);
		fflush(stdout);
	}
	//printf("Algo %s: %f réussite\n",name, (double)success/(double)nb_simul);
	float return_value = (double)success/(double)nb_simul;
	free(e.aller); free(e.retour); free(e.decalages);
	return return_value;
}


void print_python(char ** algos, int nb_algos)
{

	char buf[64];
	sprintf(buf,"plot.py");
	FILE* f_GPLT = fopen(buf,"w");
	
	if(!f_GPLT){perror("Opening python file failure\n");exit(2);}

	fprintf(f_GPLT,"import matplotlib.pyplot as plt \nimport numpy as np\nfilenames = [");
	for(int i=0;i<nb_algos;i++)
	{
		if(i<nb_algos-1)
			fprintf(f_GPLT,"'%s.data',",algos[i]);
		else
			fprintf(f_GPLT,"'%s.data']\n",algos[i]);

	}
	fprintf(f_GPLT,"labels = [");
	for(int i=0;i<nb_algos;i++)
	{
		if(i<nb_algos-1)
			fprintf(f_GPLT,"'%s',",algos[i]);
		else
			fprintf(f_GPLT,"'%s']\n",algos[i]);

	}
	fprintf(f_GPLT,"fig, ax = plt.subplots()\n");
	fprintf(f_GPLT,"for filename, label in zip(filenames, labels):\n");
	fprintf(f_GPLT,"\tdata = np.loadtxt(filename, usecols=(0, 1), unpack=True)\n");
	fprintf(f_GPLT,"\tx, y = data[0], data[1]\n");
	fprintf(f_GPLT,"\tax.plot(x, y, label=label)\n");
	fprintf(f_GPLT,"ax.set_xlabel(\"Load\")\n");
	fprintf(f_GPLT,"ax.set_ylabel(\"Success rate (%%)\")\n");
	fprintf(f_GPLT,"ax.legend(loc=\"lower left\")\n");
	fprintf(f_GPLT,"plt.xlim(0.5, 1)\n");
	fprintf(f_GPLT,"plt.savefig('result.pdf', format='pdf')\n");
   
	fclose(f_GPLT);
	

}

int main()
{
	int seed = time(NULL); 
	printf("Paramètres :\n -Periode %d\n-Nombre de routes %d\n-Taille maximum des routes %d\n-Nombre de simulations %d\n",PERIODE,NB_ROUTES,TAILLE_ROUTES,NB_SIMUL);
		//Toujours mettre exhaustivesearch en derniere
	int nb_algos = 4;
	if(EXACT_RESOLUTION)
		nb_algos++;
	struct timeval tv1, tv2;
	float running_time[nb_algos];
	char * noms[] = {"Greedy Uniform","FirstFit","Greedy Potential","Swap and Move","Exact Resolution"};
	char buf[256];
	FILE * f[nb_algos];
	FILE * time = fopen("time.data","w");
	for(int i=0;i<nb_algos;i++)
	{
		sprintf(buf,"%s.data",noms[i]);
		printf("Opening %s ...",buf);
		f[i] = fopen(buf,"w");
		if(!f[i])perror("Error while opening file\n");
		printf("OK\n");
		running_time[i]=0.0;
	}
	int nb_routes_start = FIG18?5:NB_ROUTES;
	for(int i=nb_routes_start;i<=NB_ROUTES;i++)
	{
		for(int j=0;j<nb_algos;j++)
		{
			running_time[j]=0.0;
		}
		printf("\n %d Routes \n",i);
		gettimeofday (&tv1, NULL);	
		fprintf(f[0],"%f %f\n",i/(float)NB_ROUTES,statistique(PERIODE,i, TAILLE_ROUTES,NB_SIMUL,seed,greedy_uniform,"Greedy Uniform")); //ça n'est pas sur les memes entrees a cause du rand
		gettimeofday (&tv2, NULL);	
		running_time[0] += time_diff(tv1,tv2);
		gettimeofday (&tv1, NULL);	
		fprintf(f[1],"%f %f\n",i/(float)NB_ROUTES,statistique(PERIODE,i, TAILLE_ROUTES,NB_SIMUL,seed,greedy_first_fit,"First Fit"));
		gettimeofday (&tv2, NULL);	
		running_time[1] += time_diff(tv1,tv2);
		gettimeofday (&tv1, NULL);	
		fprintf(f[2],"%f %f\n",i/(float)NB_ROUTES,statistique(PERIODE,i, TAILLE_ROUTES,NB_SIMUL,seed,greedy_potential,"Greedy Potential"));
		gettimeofday (&tv2, NULL);	
		running_time[2] += time_diff(tv1,tv2);
		//statistique(PERIODE,NB_ROUTES, PERIODE,NB_SIMUL,seed,greedy_advanced,"advanced_profit");
		//algo bugué, ne marche pas pour 50%
		gettimeofday (&tv1, NULL);	
		fprintf(f[3],"%f %f \n",i/(float)NB_ROUTES,statistique(PERIODE,i, TAILLE_ROUTES,NB_SIMUL,seed,swap,"Swap and Move"));
		gettimeofday (&tv2, NULL);	
		running_time[3] += time_diff(tv1,tv2);
		fprintf(time,"%d ",i);
		for(int i=0;i<nb_algos;i++)
		{
			//printf("Temps d'execution %s = %f \n",noms[i],running_time[i]/NB_SIMUL);
			fprintf(time,"%f ",running_time[i]);
		}
		fprintf(time,"\n");
		if(EXACT_RESOLUTION)
			fprintf(f[4],"%f %f \n",i/(float)NB_ROUTES,statistique(PERIODE,i, PERIODE,NB_SIMUL,seed,recsearch,"Exact Resolution"));
	}
	for(int i=0;i<nb_algos;i++)
	{
		
		fclose(f[i]);
	}
	fclose(time); 
	print_python(noms,nb_algos);
	return 0;
}
