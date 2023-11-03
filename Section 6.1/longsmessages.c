/***
Copyright (c) 2023 Guiraud Maël & Strozecki Yann
All rights reserved.
*///


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include<sys/time.h>
#include <omp.h>
#define MESSAGE_SIZE 1000
#define PERIOD 10000
#define DELAY_MAX 10000
#define NB_SIMULS 10000
#define PARALLEL 1
#define EXACT_RESOLUTION 1
double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}
int * random_graph(int nb_routes,int size_route)
{
	int * graph;
	assert(graph = (int*)malloc(sizeof(int)*nb_routes));
	for(int i=0;i<nb_routes;i++)
	{
		graph[i]=rand()%size_route;
	}
	return graph;
}
int* init_period(int period)
{
	int * p;
	assert(p = (int*)malloc(sizeof(int)*period));
	memset(p,0,sizeof(int)*period);
	return p;
}
int min(int a, int b){
	return (a<b)?a:b;
}
void fill_periods(int * a, int *r, int offset,int decalage, int message_size, int period)
{
	for(int i=offset;i<(offset+message_size)%period;i++)
	{
		a[i] = 1;
		r[(i+decalage)%period]=1;
	}
}
int offset_ok(int * p,int*p2, int offset, int decalage,int period, int message_size)
{
	for(int i=offset;i<(offset+message_size)%period;i++)
	{
		if(p[i])
			return 0;
		if(p2[(i+decalage) % period])
			return 0;
	}
	return 1;
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

//renvoie la distance en slot a laquelle est la fin du message avec lequel on colissione dans la periode, ou 0 si l'offset est ok
int cols_check(int *P, int offset, int message_size,int per, int nb_routes)
{
	for(int i=0;i<nb_routes;i++)
	{
		if( (P[i]+message_size)%per < P[i]%per  ) // le message pi est a cheval sur deux periodes 
		{
			if(offset%per < (P[i]+message_size)%per)
			{
				return (P[i]+message_size)%per - offset%per ;
			}	
			if(( (offset+message_size)%per > P[i]%per) ||( (offset)%per >= P[i]%per) ||( (offset+message_size)%per <= (P[i]+message_size)%per) )
			{
				return per - (offset)%per + (P[i]+message_size)%per ;
			}	
		}
		else
		{
			if((offset%per >= P[i]%per) && ( offset%per < (P[i]+message_size)%per)  )
			{
				return (P[i]+message_size)%per - offset%per ;
			}	
			if(( (offset+message_size)%per > P[i]%per) && ( (offset+message_size)%per <= (P[i]+message_size)%per) )
			{
				if(offset%per > (P[i]+message_size)%per)
				{
					return per - (offset)%per + (P[i]+message_size)%per ; 
				}
				else
				{
					return (P[i]+message_size)%per - offset%per ;
				}
			}
		}
	}

	return 0;

}

int verifie_solution(int * aller, int * retour,int message_size,int nb_routes,int period)
{
	for(int i=0;i<nb_routes;i++)
	{
		for(int j=0;j<nb_routes;j++)
		{
			if(i!=j)
			{
				if(min(abs(aller[i]-aller[j]),abs(aller[i]+period-aller[j]))<message_size)
				{
					printf(" pb aller i %d j %d val |%d - %d| = %d",i,j,aller[i],aller[j],abs(aller[i]-aller[j]));
					return 0;
				}
				if(min(abs(retour[i]-retour[j]),abs(retour[i]+period-retour[j]))<message_size)
				{
					printf(" pb retour i %d j %d val |%d - %d| = %d",i,j,retour[i],retour[j],abs(retour[i]-retour[j]));
					return 0;
				}
			}
		
		}
	}
	return 1;
}
int first_fit_core(int*aller,int*retour,int begin,int* graph,int nb_routes,int period,int message_size)
{
	int nb_routes_ok=0;
	int out;
	for(int i=0;i<nb_routes;i++)
	{
		if(graph[i] != -1)
		{
			int check_value = 1;
			int offset = 0;
			out  = 0;
			while(check_value)
			{
				check_value = cols_check(aller,offset,message_size,period,nb_routes_ok+begin);
				int tmp= check_value;
				//printf("%d ",check_value);
				check_value += cols_check(retour,offset+check_value+graph[i],message_size,period,nb_routes_ok+begin);
				//printf ("%d \n",check_value-tmp);
				offset += check_value;
				if(offset >= period)
				{
					out = 1;
					break;
				}
			}
			//si on a pas trouvé d'offset pour cette route, on passe a la suivante
			if(out)
				continue;
			aller[nb_routes_ok+begin]=offset;
			retour[nb_routes_ok+begin]=(offset+graph[i])%period;
			nb_routes_ok++;
			
		}
		
		
	}
	


	return nb_routes_ok;
}
int first_fit(int* graph,int nb_routes,int period,int message_size)
{
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int nb_ok = first_fit_core(aller,retour,0,graph,nb_routes,period,message_size);

	if(!verifie_solution(aller,retour,message_size,nb_ok,period))
	{
		printf("Error verifie solution first fit \n") ;
		exit(3);
	}
	return nb_ok;
	
}
int compact_fit(int* graph,int nb_routes,int period,int message_size)
{

	/*if(period%message_size != 0)
	{
		printf("Warning, P/tau != 0, meta offset firstfit cannot run\n");
		return 0;
	}*/
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int out;
	int cpygraph[nb_routes];
	memcpy(cpygraph,graph,sizeof(int)*nb_routes);
	int release[nb_routes];
	for(int i=0;i<nb_routes;i++)
	{
		release[i]= graph[i]%message_size;
	}
	tri_bulles(release,graph,nb_routes);
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int nb_routes_ok=0;
	for(int i=0;i<nb_routes;i++)
	{
		
		out = 0;
		for(int offset=0;offset<period;offset+= message_size)
		{
			
			if(!cols_check(aller,offset,message_size,period,nb_routes_ok) && !cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok) )
			{
			
				if(cols_check(retour,offset+graph[i]-message_size,message_size,period,nb_routes_ok)|| (nb_routes_ok == 0))
				{
					aller[nb_routes_ok]=offset;
					retour[nb_routes_ok]=(offset+graph[i])%period;
					nb_routes_ok++;
					out = 1;
				}
				
			}
			if(out)
				break;
		}
		if(!out)
		{	//Si on a pas pu agrandire la compact uple, on fait first fit meta offset
			for(int offset=0;offset<period;offset+= message_size)
			{
				
				if(!cols_check(aller,offset,message_size,period,nb_routes_ok) && !cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok) )
				{
					
					aller[nb_routes_ok]=offset;
					retour[nb_routes_ok]=(offset+graph[i])%period;
					nb_routes_ok++;
					out = 1;			
				}
				if(out)
					break;
			}
		}
	
	}	
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution MetaOffset firstfit\n") ;
		exit(4);
		}
	memcpy(graph,cpygraph,sizeof(int)*nb_routes);
	return nb_routes_ok;
	
}
int trouve_plus_proche_supp(int *t, int n, int size,int message_size,int period,int decalage, int type)
{
	int maxtmp=INT_MAX;
	int mini = INT_MAX;

	for(int i= 0;i<size;i++)
	{
		if(t[i]<mini)
			mini = t[i];
		if(t[i]-message_size>=n)
		{
			if(maxtmp > t[i]-message_size)
				maxtmp = t[i]-message_size;
		}

	}
	
	if(maxtmp == INT_MAX)
	{
		if(type == 0)//A l'aller
		{
			return period-n+mini-message_size+1;
		}
		else
		{
	
			return min(period-n+mini-message_size+1,period-n+decalage+1);
		}
		
	}

	return maxtmp-n+1;
}

int nb_lost_after(int *t, int n, int size,int message_size,int period)
{

	if(size == 0)
		return 0;
	int maxtmp=INT_MAX;
	int mini = INT_MAX;

	for(int i= 0;i<size;i++)
	{
		if(t[i]<mini)
			mini = t[i];
		if(t[i]-message_size>=n)
		{
			if(maxtmp > t[i]-message_size)
				maxtmp = t[i]-message_size;
		}

	}
	
	if(maxtmp == INT_MAX)
	{
		
		return period-n+mini-message_size;

		
	}
	return maxtmp-n;
}
int nb_lost_before(int *t, int n, int size,int message_size,int period)
{
	if(size == 0)
		return 0;
	int mintmp=-1;
	int maxi = -1;

	for(int i= 0;i<size;i++)
	{
		if(t[i]>maxi)
			maxi = t[i];
		if(t[i]+message_size<=n)
		{
			if(mintmp < t[i]+message_size)
				mintmp = t[i]+message_size;
		}


	}
	
	if(mintmp == -1)
	{
		return n+period-(maxi+message_size);
	}
	
	return n-mintmp;
}

//renvoie le nombre de tics disponible avant le prochain tic occupé dans la periode
int prochain_occup(int * aller, int * retour, int offset, int nb_routes,int period,int message_size,int decalage)
{
	if(nb_routes == 0)
		return period;
	int next_aller = trouve_plus_proche_supp(aller,offset,nb_routes,message_size,period,decalage,0);
	int next_retour = trouve_plus_proche_supp(retour,(offset+decalage)%period,nb_routes,message_size,period,decalage,1);
	return min(next_retour,next_aller);
}

int greedy_uniform(int *graph,int nb_routes,int period,int message_size)
{

	//version opti
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int nb_eligible;
	int chosen_offset;
	int old_eligible;
	int old_offset;
	int out;
	int tmp,tmp2,tmp3;
	int cpygraph[nb_routes];
	memcpy(cpygraph,graph,sizeof(int)*nb_routes);

	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int nb_routes_ok=0;
	for(int i=0;i<nb_routes;i++)
	{
		nb_eligible = 0;
		int offset = 0;
		//phase init
		tmp2=1;
		tmp3=1;
		out = 0;
		while(tmp2 || tmp3)
		{
			tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
			offset += tmp2;
			tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
			offset +=tmp3;
			if(offset >=period)
			{
				out =1;
				break;
			}
			
		}
		if(out)
		{
			continue;
		}
		while(offset < period)
		{
			//On est deja placé à une position eligible si on est ici
			tmp = prochain_occup(aller,retour,offset,nb_routes_ok,period,message_size,graph[i]);
			nb_eligible += tmp;
			offset += tmp;
			//Ici l'offset est placé tau avant un message (a l'aller ou au retour, on fait en sorte qu'il collisione avec le message)
			tmp2=1;
			tmp3=1;
			out = 0;
			while(tmp2 || tmp3)
			{
				tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
				offset += tmp2;
				tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
				offset +=tmp3;
				if(offset >=period)
				{
					out =1;
					break;
				}				
			}
			if(out)
				break;	
		}
		if(nb_eligible == 0)
		{
			continue;
		}
		chosen_offset = rand()%nb_eligible;
		nb_eligible = 0;
		offset = 0;
		tmp2=1;
		tmp3=1;
		out = 0;
		while(tmp2 || tmp3)
		{
			tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
			offset += tmp2;
			tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
			offset +=tmp3;
			if(offset >=period)
			{
				out =1;
				break;
			}
		}
		if(out)
		{
			continue;
		}
		out = 0;
		while(offset < period)
		{
			old_offset = offset;
			old_eligible = nb_eligible;
			tmp = prochain_occup(aller,retour,offset,nb_routes_ok,period,message_size,graph[i]);
			nb_eligible += tmp;
			if(nb_eligible > chosen_offset)
			{
				out = 1;
				offset = old_offset + chosen_offset- old_eligible;
				break;
			}
			offset += tmp;
			tmp2=1;
			tmp3=1;
			while(tmp2 || tmp3)
			{
				tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
				offset += tmp2;
				tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
				offset +=tmp3;
				if(offset >= period)
				{
					printf("Se poser la question si c'est normal que ca arrive\n");
					break;
				}
			}
			
		}
		if(out)
		{	
			aller[nb_routes_ok]=offset%period;
			retour[nb_routes_ok]=(offset+graph[i])%period;
			nb_routes_ok++;
			graph[i]=-1;
		}
		else
		{
			printf("%d %d Ca ne devrait pas arriver, si on avait un offset eligible on devrait l'avoir retrouvé\n ",offset,nb_eligible);
			exit(2);
		}
		
	}
	if( first_fit_core(aller,retour,nb_routes_ok,graph,nb_routes,period,message_size))
	{
		printf("First fit arrive a rajouter une route, c'est étrange (RandomOffset). \n");
		
		exit(4);
	}
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution RandomOffset\n") ;
		exit(3);
	}
	memcpy(graph,cpygraph,sizeof(int)*nb_routes);
	return nb_routes_ok;
}
typedef struct{
	int offset;
	int lost;
} Losttics;
int min_tics_lost(Losttics * tab, int size)
{
	int min = INT_MAX;
	int offset = INT_MAX;
	for(int i=0;i<size;i++)
	{
		if(tab[i].lost < min)
		{
			min = tab[i].lost;
			offset = tab[i].offset;
		}
	}
	if(offset == INT_MAX)
	{
		printf("On à pas trouvé de valeur min dans le tableau de tics lost, ce n'est pas normal.\n");
		exit(7);
	}
	return offset;
}
int super_compact(int *graph,int nb_routes,int period,int message_size)
{

	//version opti
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int nb_eligible;
	int chosen_offset;
	int old_eligible;
	int old_offset;
	int out;
	int tmp,tmp2,tmp3;
	int idlost = 0;
	Losttics tablost[nb_routes*4];
	int cpygraph[nb_routes];
	memcpy(cpygraph,graph,sizeof(int)*nb_routes);
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int nb_routes_ok=0;
	for(int i=0;i<nb_routes;i++)
	{
		idlost = 0;
		nb_eligible = 0;
		int offset = 0;
		//phase init
		tmp2=1;
		tmp3=1;
		out = 0;
		while(tmp2 || tmp3)
		{
			tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
			offset += tmp2;
			tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
			offset +=tmp3;
			if(offset >=period)
			{
				out =1;
				break;
			}
		}
		if(out)
		{
			continue;
		}		
		while(offset < period)
		{
			
			tablost[idlost].offset = offset;
			tablost[idlost].lost = min(message_size,nb_lost_before(aller,offset,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_after(aller,offset,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_before(retour,(offset+graph[i])%period,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_after(retour,(offset+graph[i])%period,nb_routes_ok,message_size,period));
			idlost++;
			//On est deja placé à une position eligible si on est ici
			tmp = prochain_occup(aller,retour,offset,nb_routes_ok,period,message_size,graph[i]);
			offset += tmp-1;
			tablost[idlost].offset = offset;
			tablost[idlost].lost = min(message_size,nb_lost_before(aller,offset,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_after(aller,offset,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_before(retour,(offset+graph[i])%period,nb_routes_ok,message_size,period))
								  +min(message_size,nb_lost_after(retour,(offset+graph[i])%period,nb_routes_ok,message_size,period));
			idlost++;
			//Ici l'offset est placé tau avant un message (a l'aller ou au retour, on fait en sorte qu'il collisione avec le message)
			offset += message_size;
			tmp2=1;
			tmp3=1;
			out = 0;
			while(tmp2 || tmp3)
			{
				tmp2 = cols_check(aller,offset,message_size,period,nb_routes_ok);
				offset += tmp2;
				
				tmp3 =   cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
				offset +=tmp3;
		
				if(offset >=period)
				{
					out =1;
					break;
				}
				
			}
			if(out)
				break;
		}
		offset = min_tics_lost(tablost,idlost);
		aller[nb_routes_ok]=offset%period;
		retour[nb_routes_ok]=(offset+graph[i])%period;
		nb_routes_ok++;
		graph[i]=-1;	
	}
	if( first_fit_core(aller,retour,nb_routes_ok,graph,nb_routes,period,message_size))
	{
		printf("First fit arrive a rajouter une route, c'est étrange (SuperCompact). \n");
		
		exit(6);
	}
	int indice_r=0;
	for(int i=0;i<nb_routes;i++)
	{
		//printf("%d %d \n",graph[i],cpygraph[i]);
		if(graph[i] == -1)
		{
			//printf("%d %d \n",aller[indice_r],retour[indice_r]);
			
			if((aller[indice_r]+cpygraph[i])%period != retour[indice_r])
			{
				printf("Probleme\n");
				exit(10);
			}
			indice_r++;
		}
			
		
	}
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution SuperCompact\n") ;
		exit(5);
	}
		memcpy(graph,cpygraph,sizeof(int)*nb_routes);

	return nb_routes_ok;
}
int meta_offset(int *graph,int nb_routes,int period,int message_size)
{
	/*if(period%message_size != 0)
	{
		printf("Warning, P/tau != 0, meta offset cannot run\n");
		return 0;
	}*/
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int out;
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int nb_routes_ok=0;
	for(int i=0;i<nb_routes;i++)
	{
		out = 0;
		for(int offset=0;offset<period;offset+= message_size)
		{
			if(!cols_check(aller,offset,message_size,period,nb_routes_ok) && !cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok) )
			{
				aller[nb_routes_ok]=offset;
				retour[nb_routes_ok]=(offset+graph[i])%period;
				nb_routes_ok++;
				out = 1;
			}
			if(out)
				break;
		}
	
	}	
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution MetaOffset\n") ;
		exit(4);
		}
	
	return nb_routes_ok;
}
int shortestlongest(int *graph,int nb_routes,int period,int message_size)
{
	/*if(period%message_size != 0)
	{
		printf("Warning, P/tau != 0, meta offset cannot run\n");
		return 0;
	}*/
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int budget = period-nb_routes*message_size;
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	int release[nb_routes];
	for(int i=0;i<nb_routes;i++)
	{
		release[i]= graph[i];
	}

	tri_bulles(graph,release,nb_routes);
	int nb_routes_ok=0;
	int offset = 0;
	for(int i=0;i<nb_routes;i++)
	{
		if(i>0)
		{

			budget -= release[i]-release[i-1];

			if(budget < 0)
			{
				break;
			}
		}
		offset = i*message_size;
		aller[nb_routes_ok]=offset;
		retour[nb_routes_ok]=(offset+release[i])%period;
		nb_routes_ok++;
		

	
	}	
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution ShortestLongest\n") ;
		exit(4);
		}
	
	return nb_routes_ok;
}
//check if two messages are a pair or not
int check_pair(int di, int dj,int message_size,int period)
{
	int a = di/message_size;
	int b = dj/message_size;
	int c = period/message_size;

	if((b-a==1) || (b-a==-(c-1)))
	{
		return 0;
	}
	return 1;
}
int mod(int a, int b)
{
	int res = a%b;
	if(res >= 0)
	{
		return res;
	}
	else
	{
		while(res < 0)
		{
			res += b;
		}
		return res;
	}
}
int compact_pair(int *graph,int nb_routes,int period,int message_size)
{
	int cpygraph[nb_routes];
	memcpy(cpygraph,graph,sizeof(int)*nb_routes);
	int release[nb_routes];
	for(int i=0;i<nb_routes;i++)
	{
		release[i]= graph[i]%message_size;
	}

	tri_bulles(release,graph,nb_routes);
	int cpygraphsorted[nb_routes];
	memcpy(cpygraphsorted,graph,sizeof(int)*nb_routes);
	int nb_routes_ok = 0;
	//Le graph est trié en fonction des releases
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int chkpair;
	int out;
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	for(int i=0;i<nb_routes-1;i+=2)
	{
		chkpair = 0;
		while(!check_pair(graph[i],graph[i+1],message_size,period))
		{
			i++;
			if(i >nb_routes - 2)
			{
				chkpair = 1;
				break; 
			}
		}
		//Fin des routes.
		if(chkpair == 1)
			break;
		out = 0;
		for(int offset = 0 ;offset < period;offset+= message_size)
		{
			int offset2 = mod(offset+graph[i]+message_size + graph[i+1]%message_size-graph[i]%message_size-graph[i+1],period);
			if(!cols_check(aller,offset,message_size,period,nb_routes_ok) && !cols_check(aller,offset2,message_size,period,nb_routes_ok) &&
			   !cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok) && !cols_check(retour,offset2+graph[i+1],message_size,period,nb_routes_ok))
			{
				aller[nb_routes_ok]=offset;
				retour[nb_routes_ok]=(offset+graph[i])%period;
				nb_routes_ok++;
				aller[nb_routes_ok]=offset2%period;
				retour[nb_routes_ok]=(offset2+graph[i+1])%period;
				nb_routes_ok++;
				graph[i]=-1;
				graph[i+1]=-1;
				out = 1;
			}
			if(out)
				break;
		}
	}

	int indice_r=0;
	for(int i=0;i<nb_routes;i++)
	{
		//printf("%d %d \n",graph[i],cpygraphsorted[i]);
		if(graph[i] == -1)
		{
			//printf("%d %d \n",aller[indice_r],retour[indice_r]);
			
			if( (aller[indice_r]+cpygraphsorted[i])%period != retour[indice_r])
			{
				printf("Probleme pairs\n");
				exit(10);
			}
			indice_r++;
		}
			
		
	}

	for(int i=0;i<nb_routes;i++)
	{
		if(graph[i] != -1)
		{
			out = 0;
			for(int offset=0;offset<period;offset+= message_size)
			{
				if(!cols_check(aller,offset,message_size,period,nb_routes_ok) && !cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok) )
				{
					aller[nb_routes_ok]=offset;
					retour[nb_routes_ok]=(offset+graph[i])%period;
					nb_routes_ok++;
					out = 1;
				}
				if(out)
					break;
			}
		}
		
	
	}	
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution MetaOffsetPair\n") ;
		exit(8);
	}

	memcpy(graph,cpygraph,sizeof(int)*nb_routes);
	return nb_routes_ok;
}
int pair(int *graph,int nb_routes,int period,int message_size)
{
	int cpygraph[nb_routes];
	memcpy(cpygraph,graph,sizeof(int)*nb_routes);
	int release[nb_routes];
	for(int i=0;i<nb_routes;i++)
	{
		release[i]= graph[i]%message_size;
	}
	tri_bulles(release,graph,nb_routes);
	int nb_routes_ok = 0;
	//Le graph est trié en fonction des releases
	int aller[nb_routes] ;
	int retour[nb_routes] ;
	int chkpair;
	for(int i=0;i<nb_routes;i++)
	{
		aller[i]=0;
		retour[i]=0;
	}
	for(int i=0;i<nb_routes-1;i+=2)
	{
		chkpair = 0;
		while(!check_pair(graph[i],graph[i+1],message_size,period))
		{
			i++;
			if(i >nb_routes - 2)
			{
				chkpair = 1;
				break; 
			}
		}
		//Fin des routes.
		if(chkpair == 1)
			break;
		int check_value = 1;
		int check_value2 = 1;
		int offset = 0;
		int out  = 0;
		int offset2;
		while(check_value || check_value2)
		{
			offset2 = mod(offset+graph[i]+message_size + graph[i+1]%message_size-graph[i]%message_size-graph[i+1],period);
		
			check_value = cols_check(aller,offset,message_size,period,nb_routes_ok);
			check_value += cols_check(aller,offset2,message_size,period,nb_routes_ok);
			offset += check_value ;
			offset2 = mod(offset+graph[i]+message_size + graph[i+1]%message_size-graph[i]%message_size-graph[i+1],period);
	
			check_value2 = 1;
			while(check_value2)
			{
				check_value2 = cols_check(retour,offset+graph[i],message_size,period,nb_routes_ok);
				if(!check_value2)
				{
					check_value2 = cols_check(retour,offset2+graph[i+1],message_size,period,nb_routes_ok);
				}
				offset += check_value2;
				offset2 = mod(offset+graph[i]+message_size + graph[i+1]%message_size-graph[i]%message_size-graph[i+1],period);
				
				if(offset >= period)
				{
					out = 1;
					break;
				}
			}
			
		
			if(offset >= period)
			{
				out = 1;
				break;
			}
		}
		//si on a pas trouvé d'offset pour cette route, on passe a la suivante
		if(out)
			continue;

		offset2 = mod(offset+graph[i]+message_size + graph[i+1]%message_size-graph[i]%message_size-graph[i+1],period);
		aller[nb_routes_ok]=offset;
		retour[nb_routes_ok]=(offset+graph[i])%period;
		nb_routes_ok++;
		aller[nb_routes_ok]=offset2%period;
		retour[nb_routes_ok]=(offset2+graph[i+1])%period;
		nb_routes_ok++;

		graph[i]=-1;
		graph[i+1]=-1;


	}

	//Fin des paires, on lance le first fit
	nb_routes_ok += first_fit_core(aller,retour,nb_routes_ok,graph,nb_routes,period,message_size);
	
	if(!verifie_solution(aller,retour,message_size,nb_routes_ok,period))
	{
		printf("Error verifie solution pairs\n") ;
		exit(8);
	}

	memcpy(graph,cpygraph,sizeof(int)*nb_routes);
	return nb_routes_ok;
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
	fprintf(f_GPLT,"plt.savefig('result.pdf', format='pdf')\n");   
	fclose(f_GPLT);
	

}


//Pour le new bruteforce
typedef struct{
	int *next;
	int margin;
}stack;



void print_solution(int *id, int *start_slot, int *return_slot,  int size, stack *fw, stack *bw){
	printf("Partial solution of size %d:\n", size);
	printf("Forward windows  (forward margin : %d): ",fw[size].margin);
	for(int i = 0; i <= size; i++){
		printf("[%d :(%d,%d)] ",id[i], start_slot[i],fw[size].next[i]);
	}
	printf("\n");
	printf("Backward windows  (backward margin : %d): ",bw[size].margin);
	for(int i = 0; i <= size; i++){
		printf("[%d :(%d,%d)] ",id[i], return_slot[i],bw[size].next[i]);
	}
	printf("\n");
}

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


int exact_resolution(int * return_time,int route_number, int period,int message_size){
	


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

int main(int argc,char * argv[])
{
	int* graph;

	int nb_simuls = NB_SIMULS;
	int message_size = MESSAGE_SIZE;
	
	
	int size_route = DELAY_MAX;
	srand(time(NULL));
	struct timeval tv1, tv2;
	int nb_algos = 5;
	if(EXACT_RESOLUTION)
		nb_algos++;
	int tmp[nb_algos];
	float running_time[nb_algos]; 
		//Toujours mettre exhaustivesearch en derniere
	char * noms[] = {"FirstFit","MetaOffset","GreedyUniform","CompactPairs","CompactFit","ExactResolution"};
	char buf[256];
	FILE ** f = malloc(sizeof(FILE*)*nb_algos);
	float success[nb_algos];
	for(int i=0;i<nb_algos;i++)
	{
		sprintf(buf,"%s.data",noms[i]);
		printf("Opening %s ...",buf);
		f[i] = fopen(buf,"w");
		if(!f[i])perror("Error while opening file\n");
		success[i]=0.0;
		printf("OK\n");
		running_time[i] = 0.0;
	}
	int period = PERIOD;
	for(int nb_routes = (period/message_size)*0.5 ; nb_routes<=period/message_size;nb_routes+=1)
	{
		for(int algo=0;algo<nb_algos;algo++)
		{
			success[algo]= 0.0;

		}
		#pragma omp parallel for private(graph,tmp)  if (PARALLEL)
		for(int j=0;j<nb_simuls;j++)
		{
			fprintf(stdout,"\r Computing load %f :  %d/%d",(float)nb_routes*message_size/period,j+1,nb_simuls);fflush(stdout);
			graph = random_graph(nb_routes,size_route);
			pair(graph,nb_routes,period,message_size);
				
			
			for(int algo = 0;algo<nb_algos;algo++)
			{
				gettimeofday (&tv1, NULL);	
				switch(algo){
					case 0:
					tmp[algo] =  first_fit(graph,nb_routes,period,message_size);
					
					break;
					case 1:
						
						tmp[algo] = meta_offset(graph,nb_routes,period,message_size);
						break;
					case 2: 
						tmp[algo] = greedy_uniform(graph,nb_routes,period,message_size);

						break;
					case 3:
						tmp[algo] = compact_pair(graph,nb_routes,period,message_size);
						break;
					
						
					case 4: 
						tmp[algo] = compact_fit(graph,nb_routes,period,message_size);
						break;
					case 5:
						tmp[algo] = exact_resolution(graph,nb_routes,period,message_size);
						break;
					}
					gettimeofday (&tv2, NULL);	
					#pragma omp critical
					running_time[algo] += time_diff(tv1,tv2);
		
					
					if(tmp[algo] == nb_routes)
					{
						#pragma omp atomic
							success[algo]++;
					}
					
					
					
						
			}
			free(graph);
			
		}
		
		for(int algo = 0;algo<nb_algos;algo++)
		{
			fprintf(f[algo], "%f ",((float)message_size*nb_routes)/period);
			fprintf(f[algo],"%f ",success[algo]/nb_simuls *100);
			fprintf(f[algo],"\n");
			
		}
		
	}
	for(int algo = 0;algo<nb_algos;algo++)fclose(f[algo]);
	print_python( noms,  nb_algos);
	free(f);
	return 0;
	
}
