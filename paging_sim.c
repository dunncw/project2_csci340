#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct job {
    // the number of the job
    int number;
    int mem;
    //number of pages a job will need to take up in the mem
    int pn;
    int run_time;
    // the time left before the job is done
    int time_to_fin;
    // if teh job is done 1 if not 0
    int done;
    // if the job is present on the mem 1 of not 0 
    int present;
    //this is the response time of the job
    int start;
    // this is turn around time of teh job
    int fin;
    struct job *next;
};
typedef struct job job;

struct queue {
    //the number of jobs in queue
    int count;
    // if all jobs are done this is a 1 if not 0
    int done;
    //pointer to both first an last job of the queue
    job *front;
    job *rear;
};
typedef struct queue queue;

struct page_table_entry{
    // the page frame number that a pf uses to point to a place in physical mem
    int pfn;
    // if a job is done it can be marked done with a 1 if not done its a 0
    int done;
    //this keeps track of what job is currently occupying the page frame if -1 then no job is
    int jo;
};
typedef struct page_table_entry page_table_entry;

//simple void method that inits the queue data struct used to hold the jobs 
void initialize_queue(queue *q)
{
    q->count = 0;
    q->done = 0;
    q->front = NULL;
    q->rear = NULL;
}

// find a random number within the given range passed to it and return said number 
int rand_inRange(int l, int u, int mode, int page_sz){
    // this has 2 modes because u need 2 dif kinds of random num's 
        //mode 0, i want a rand num within a range that is not an even multiple of the page sz
        //mode 1, i want a rand num within a range that is an even muktipe of the page sz
    if(mode == 0){
        int re_val = (rand() % (u - l + 1)) + l;
        //test print ot see what this method is returing 
        //printf("upper <%d> lower<%d> re_val<%d>\n", u, l, re_val);
        return re_val;
    }
    if(mode == 1){
        int re_val = (rand() % (u - l + 1)) + l;
        while(re_val % page_sz != 0)
            re_val = (rand() % (u - l + 1)) + l;

        //test print ot see what this method is returing 
        //printf("upper <%d> lower<%d> re_val<%d>\n", u, l, re_val - page_sz);
        return re_val;
    }
    //if -1 is returned somthing when wrong
    return -1;
}

// this medthod puts all the random jobs in the queue
void enqueue_a_job(queue *q, int lm, int um, int lr, int ur, int i, int page_sz)
{
    // create a new job to be put in queue
    job *j;
    j = malloc(sizeof(job));
    j->number = i;
    int num = rand_inRange(lm, um, 1, page_sz);
    j->mem = num;
    j->pn =num/page_sz;
    j->run_time = rand_inRange(lr, ur, 0, page_sz);
    j->time_to_fin = j->run_time;
    j->done = 0;
    j->present = 0;
    j->next = NULL;

    //test print to see all the feilds of a job are being initilized and have correct values 
    //printf("jfeilds: mem <%d> rt<%d> ttf<%d> d<%d> p<%d> n<%d>\n", j->mem, j->run_time, j->time_to_fin, j->done, j->present, j->number); 


    //the queue is not empty so add the job to it 
    if(q->rear != NULL)
    {
        //sets the jobs next to link the items in the queue together
        q->rear->next = j;
        q->rear = j;
    }
    //there is no job so this is the first job in the queue 
    else
    {
        q->front = q->rear = j;
    }
    //incfeaase count of queue
    q->count++;
    

}

// this shows the contente of the queue and the specifics of each job 
void display_q(job *head)
{
    if(head == NULL)
    {
        printf("\n");
    }
    else
    {
        printf("Job %d: mem <%d> rt <%d> ttf<%d>\n", head->number, head->mem, head->run_time, head->time_to_fin);
        display_q(head->next);
    }
}

// this method differs from the one above as it displays the runtime and turn around time and reponse time of the jobs after they have already finsihed
void display_q_end(job *head)
{
    if(head == NULL)
    {
        printf("\n");
    }
    else
    {
        printf("Job %d: mem <%d> start time<%d> end time<%d>\n", head->number, head->mem, head->start, head->fin);
        display_q_end(head->next);
    }
}

//this method inits the page table and sets the page frame number, job occupying it, and if the job is done or not
void pt_init(page_table_entry ** pt, int num_pgs){
    for(int i = 0; i < num_pgs; i++ ){

        page_table_entry *pfe;
        pfe = malloc(sizeof(page_table_entry));

        pfe->pfn = i;
        pfe->jo = -1;
        pfe->done = 0;

        pt[i] = pfe;
        
        //test print statment to make sure the page table is being init'ed correctly
        //printf("%d\t%d\t%d\n", pfe->pfn, pfe->jo, pfe->done);
    }
}

// this method displays the contents of the page table 
void display_pt(page_table_entry **pt, int num_pgs){
    
    for(int i = 0; i < num_pgs; i = i + num_pgs/2 )
    {
            printf("\t");
        for(int a = 0; a < num_pgs/2; a++){
            page_table_entry *temp = pt[i + a];
        
            printf("%d ",temp->jo);
        }
        printf("\n");
    }
    printf("\n");
}

// this method is a helper method of round_robin and checks to see if all the jobs in the queue are done
int fin(job *head, int count){

    for(int i = 0; i < count; i++){
        if(head->done != 1)
            return 0;
        head = head->next;
    }
    return 1;
    
}

// this method runs the round robin
void round_robin(job *head, page_table_entry **pt, int num_pgs, int open_space, queue *q, int time_slice){

    // check to see if the job is done
    if(head->done == 0){
        if(head->present == 1 && head->done == 0){// if the the job had been updated to present and is not done then let it run on the cpu
            printf("time slice: %d\n", time_slice);
            head->time_to_fin = head->time_to_fin - 1; // update teh jobs ttf
            if(head->time_to_fin == 0){ // if teh job is done then
                head->fin = time_slice; // mark the time it finished at
                head->done = 1;//mark the job done
                // loop thru the page table and mark all the pages taht are share the same number as the job taht just complelted as done
                for(int i = 0; i < head->pn; i++){
       
                    page_table_entry *temp = pt[i];

                    if(temp->jo == head->number){
                        temp->done = 1;
                        //make sure to update counter taht these new pages are open and aviabalebe to be used
                        open_space++;
                    }
                    
                } 
            }
            // add on to the time slice
            time_slice = time_slice + 1;
            printf("    Ran job<%d> on cpu: rt<%d> ttf<%d> done<%d>\n    Page Table Contents:\n", head->number, head->run_time, head->time_to_fin, head->done);
            display_pt(pt, num_pgs);
        }

        // if the job is not already in mem then load it into mem
        if(head->pn <= open_space && head->present == 0){
            printf("Loaded job %d into mem\n", head->number);
            head->start = time_slice;
            int pn = head->pn;
            //allocate teh pages need for teh program in the mem
            for(int i = 0; i < num_pgs; i++){
                if(pn > 0){
                    
                    page_table_entry *temp = pt[i];

                    if(temp->jo == -1 || temp->done == 1){
                        temp->jo = head->number;
                        // make sure to count how many pages are taken up by the program
                        pn--;
                    } 
                }     
            }
            //mark the program present on mem and count open space left in mem
            head->present = 1;
            open_space = open_space - head->pn;
        }
    }
    
    // if all the jobs are done stop but if not keep going on round robin
    if(fin(q->front, q->count) == 0) {
        //if at end of q go to start 
        if(head->next == NULL){
            round_robin(q->front, pt, num_pgs, open_space, q, time_slice);
            }
        //otherwise go to next job
        else{
            round_robin(head->next, pt, num_pgs, open_space, q, time_slice);
        }  
    }

} 

int main(int argc, char *argv[])
{
    if (argc < 3) {
	fprintf(stderr, "usage: page_sim <seed> <comp_mem_sz> <page_sz> <number_of_jobs> <min_run_tiem> <max_run_time> <min_mem> <max_mem>\n");
	exit(1);
    }
    // take all the stuff from the command line and put em in variables 
    int seed = atoi(argv[1]);
    int comp_mem_sz  = atoi(argv[2]);
    int page_sz = atoi(argv[3]);
    int number_of_jobs = atoi(argv[4]);
    int min_run_time = atoi(argv[5]);
    int max_run_time = atoi(argv[6]);
    int min_mem  = atoi(argv[7]);
    int max_mem = atoi(argv[8]);


    //set random to the seed so that the random numbers generated can be regenerated again for testing purposes 
    srand(seed);

    //check to make sure mem size is an even multiple of the page size
    int div = comp_mem_sz % page_sz;
    printf("%d\n", div);
    if(comp_mem_sz % page_sz != 0){
        printf("comp_mem is not an even multiple of page size: (comp_mem)<%d> mod (page_sz)<%d> = <%d>\n", comp_mem_sz, page_sz, div);
        exit(1);
    }  

    //make a queue data structer for the jobs to reside in
    queue *q;
    q = malloc(sizeof(queue));
    initialize_queue(q);
    
    
    //make the jobs given the paramters passed 
    for(int i = 0; i < number_of_jobs; i++){
        enqueue_a_job(q, min_mem, max_mem, min_run_time, max_run_time, i, page_sz);
    }
        

    // how many pages are their
    //make a page table of this size
    //init the new page tabel
    int num_pgs = comp_mem_sz/page_sz;

    page_table_entry *pt[num_pgs];

    pt_init(pt, num_pgs);


    // count how much open space there is in mem
    int open_space = num_pgs;

    //start the round robin

    printf("simulator paramaters:\n\tmemory: %d\n \tpage size: %d\n \tSeed: %d\n \tnumbers of jobs: %d\n \truntime (min-max): %d-%d\n \tmemory: %d-%d\n\n", comp_mem_sz, page_sz, seed, number_of_jobs, min_run_time, max_run_time, min_mem, max_mem);
    
    printf("Job queue:\n");
    display_q(q->front);
    printf("starting sim\n");
    round_robin(q->front, pt, num_pgs, open_space, q, 1);

    printf("job info:\n");
    display_q_end(q->front);

    printf("End Sim\n");
    
        

    return 0;
}
