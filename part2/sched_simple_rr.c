/*
 * SCHED_SIMPLE_RR scheduling class. Implements a round robin scheduler with no
 * priority mechanism.
 */

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */
static void update_curr_simple_rr(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	u64 delta_exec;

	if (!task_has_simple_rr_policy(curr))
		return;

	delta_exec = rq->clock - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	curr->se.exec_start = rq->clock;
	cpuacct_charge(curr, delta_exec);
}

/*
 * Adding/removing a task to/from a priority array:
 */
static void enqueue_task_simple_rr(struct rq *rq, struct task_struct *p, int wakeup, bool b)
{
	// not yet implemented
	// ...

	list_add_tail(&(p->simple_rr_list_item), &(rq->simple_rr.queue)); // add 12.27
        (rq->simple_rr.nr_running)++; // add 12.27
}

static void dequeue_task_simple_rr(struct rq *rq, struct task_struct *p, int sleep)
{
	// first update the task's runtime statistics
	update_curr_simple_rr(rq);

	// not yet implemented
	// ...

	list_del(&(p->simple_rr_list_item)); // add 12.27
        (rq->simple_rr.nr_running)--; // add 12.27
}

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_simple_rr(struct rq *rq, struct task_struct *p)
{
	list_move_tail(&p->simple_rr_list_item, &rq->simple_rr.queue);
}

/*
 * current process is relinquishing control of the CPU
 */
static void
yield_task_simple_rr(struct rq *rq)
{
	// not yet implemented
	// ...

	struct task_struct *curr = rq->curr;  //add 12.27
        requeue_task_simple_rr(rq, curr);     //add 12.27
}

/*
 * Preempt the current task with a newly woken task if needed:
 * int wakeflags added to match function signature of other schedulers
 */
static void check_preempt_curr_simple_rr(struct rq *rq, struct task_struct *p, int wakeflags)
{
}

/*
 * select the next task to run
 */
static struct task_struct *pick_next_task_simple_rr(struct rq *rq)
{
	//struct task_struct *next;
	//struct list_head *queue;
	//struct simple_rr_rq *simple_rr_rq;

	// not yet implemented
	// ...

	if (list_empty(&(rq->simple_rr.queue))) return NULL; // add 12.27
	
	/* you need to return the selected task here */

	struct task_struct* next_task = list_first_entry(&rq->simple_rr.queue, struct task_struct, simple_rr_list_item);   // add 12.27
       
        next_task->se.exec_start = rq->clock; //add 12.27
        return next_task;   // add 12.27

	// return NULL;  //original
}

static void put_prev_task_simple_rr(struct rq *rq, struct task_struct *p)
{
	update_curr_simple_rr(rq);
	p->se.exec_start = 0;
}

#ifdef CONFIG_SMP
/*
 * Load-balancing iterator. Note: while the runqueue stays locked
 * during the whole iteration, the current task might be
 * dequeued so the iterator has to be dequeue-safe. Here we
 * achieve that by always pre-iterating before returning
 * the current task:
 */
static struct task_struct *load_balance_start_simple_rr(void *arg)
{
	struct rq *rq = arg;
	struct list_head *head, *curr;
	struct task_struct *p;

	head = &rq->simple_rr.queue;
	curr = head->prev;

	p = list_entry(curr, struct task_struct, simple_rr_list_item);

	curr = curr->prev;

	rq->simple_rr.simple_rr_load_balance_head = head;
	rq->simple_rr.simple_rr_load_balance_curr = curr;

	return p;
}

static struct task_struct *load_balance_next_simple_rr(void *arg)
{
	struct rq *rq = arg;
	struct list_head *curr;
	struct task_struct *p;

	curr = rq->simple_rr.simple_rr_load_balance_curr;

	p = list_entry(curr, struct task_struct, simple_rr_list_item);
	curr = curr->prev;
	rq->simple_rr.simple_rr_load_balance_curr = curr;

	return p;
}

static unsigned long
load_balance_simple_rr(struct rq *this_rq, int this_cpu, struct rq *busiest,
		unsigned long max_load_move,
		struct sched_domain *sd, enum cpu_idle_type idle,
		int *all_pinned, int *this_best_prio)
{
	struct rq_iterator simple_rr_rq_iterator;

	simple_rr_rq_iterator.start = load_balance_start_simple_rr;
	simple_rr_rq_iterator.next = load_balance_next_simple_rr;
	/* pass 'busiest' rq argument into
	 * load_balance_[start|next]_simple_rr iterators
	 */
	simple_rr_rq_iterator.arg = busiest;

	return balance_tasks(this_rq, this_cpu, busiest, max_load_move, sd,
			     idle, all_pinned, this_best_prio, &simple_rr_rq_iterator);
}

static int
move_one_task_simple_rr(struct rq *this_rq, int this_cpu, struct rq *busiest,
		 struct sched_domain *sd, enum cpu_idle_type idle)
{
	struct rq_iterator simple_rr_rq_iterator;

	simple_rr_rq_iterator.start = load_balance_start_simple_rr;
	simple_rr_rq_iterator.next = load_balance_next_simple_rr;
	simple_rr_rq_iterator.arg = busiest;

	return iter_move_one_task(this_rq, this_cpu, busiest, sd, idle,
				  &simple_rr_rq_iterator);
}
#endif

/*
 * task_tick_simple_rr is invoked on each scheduler timer tick.
 */
static void task_tick_simple_rr(struct rq *rq, struct task_struct *p,int queued)
{
	struct task_struct *curr;
	struct simple_rr_rq *simple_rr_rq;
	
	// first update the task's runtime statistics
	update_curr_simple_rr(rq);

	// not yet implemented
	//...
	
	if (--p->task_time_slice > 0)    // add 12.27
                return;			 // add 12.27
	
       	//p->task_time_slice = simple_rr_time_slice;  // add 12.27
        p->task_time_slice = p->weighted_time_slice; // add 12.31
	set_tsk_need_resched(p);		    // add 12.27
        requeue_task_simple_rr(rq, p);
	return ;		                    // add 12.27		

	// return NULL;   // original 	
}

/*
 * scheduling policy has changed -- update the current task's scheduling
 * statistics
 */
static void set_curr_task_simple_rr(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock;
}

/*
 * We switched to the sched_simple_rr class.
 */
static void switched_to_simple_rr(struct rq *rq, struct task_struct *p,
			     int running)
{
	/*
	 * Kick off the schedule if running, otherwise just see
	 * if we can still preempt the current task.
	 */
	if (running)
		resched_task(rq->curr);
	else
		check_preempt_curr(rq, p, 0);
}

static int
select_task_rq_simple_rr(struct rq *rq, struct task_struct *p, int sd_flag, int flags)
{
	if (sd_flag != SD_BALANCE_WAKE)
		return smp_processor_id();

	return task_cpu(p);
}

const struct sched_class simple_rr_sched_class = {
	.next			= &idle_sched_class,
	.enqueue_task		= enqueue_task_simple_rr,
	.dequeue_task		= dequeue_task_simple_rr,
	.yield_task		= yield_task_simple_rr,

	.check_preempt_curr	= check_preempt_curr_simple_rr,

	.pick_next_task		= pick_next_task_simple_rr,
	.put_prev_task		= put_prev_task_simple_rr,

#ifdef CONFIG_SMP
	.load_balance		= load_balance_simple_rr,
	.move_one_task		= move_one_task_simple_rr,
#endif

	.switched_to  = switched_to_simple_rr,
	.select_task_rq = select_task_rq_simple_rr,

	.set_curr_task          = set_curr_task_simple_rr,
	.task_tick		= (void *)task_tick_simple_rr,
};
