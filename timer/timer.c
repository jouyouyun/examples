#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGALRM
#define TIMER_DURATION 1000 // 600ms

static int start_signal_timer(timer_t *id, int sig, int duration);
static int start_thread_timer(timer_t *id, int duration);
static int do_start_timer(timer_t *id, struct sigevent *sev, int duration);
static int stop_timer(timer_t *id);
static void thread_handler(union sigval);
static int setup_signal(int sig);
static void signal_handler(int sig, siginfo_t *si, void *data);

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <signal/thread>\n", argv[0]);
		return -1;
	}
	timer_t timerid;
	int ret = 0;
	if (strcmp(argv[1], "signal") == 0) {
		ret = setup_signal(SIG);
		if (ret == -1) {
			printf("Failed to setup signal: %s\n", strerror(errno));
			return -1;
		}
		ret = start_signal_timer(&timerid, SIG, TIMER_DURATION);
	} else if (strcmp(argv[1], "thread") == 0 ){
		ret = start_thread_timer(&timerid, TIMER_DURATION);
	}
	if (ret == -1) {
		return -1;
	}
	printf("Create timer id: %p\n", timerid);
	printf("Please input 'quit' to exit...\n");

	char buf[1024] = {0};
	while (1) {
		printf("Please input: ");
		scanf("%s", buf);
		if (strcmp(buf, "quit") == 0) {
			break;
		}
	}

	ret = stop_timer(&timerid);
	if (ret == -1) {
		printf("Failed to delete timer: %s\n", strerror(errno));
	}
	printf("Exit...\n");
	return 0;
}

static int
start_signal_timer(timer_t *id, int sig, int duration)
{
	struct sigevent sev;

	// handle in thread when timeout
	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = sig;
	sev.sigev_value.sival_int = 111;
	return do_start_timer(id, &sev, duration);
}

static int
start_thread_timer(timer_t *id, int duration)
{
	struct sigevent sev;

	// handle in thread when timeout
	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = thread_handler;
	sev.sigev_value.sival_int = 111;
	return do_start_timer(id, &sev, duration);
}

static int
do_start_timer(timer_t *id, struct sigevent *sev, int duration)
{
	struct itimerspec its; // duration settings

	int ret = timer_create(CLOCKID, sev, id);
	if (ret == -1) {
		printf("Failed to create timer: %s\n", strerror(errno));
		return -1;
	}
	printf("The timer id: %p\n", id);

	// set timeout, only once
	// it_value the first timeout duration
	// it_interval the next timeout duration
	if (duration >= 1000) {
		its.it_value.tv_sec = duration / 1000;
		its.it_value.tv_nsec = (duration%1000) * 1000000;
	} else {
		its.it_value.tv_nsec = duration * 1000000;
	}
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	ret = timer_settime(*id, 0, &its, NULL);
	if (ret == -1) {
		printf("Failed to set timeout: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

static int
stop_timer(timer_t *id)
{
	if (*id == 0) {
		return 0;
	}
	return timer_delete(*id);
}

static void
thread_handler(union sigval v)
{
	printf("Timer arrived: %d\n", v.sival_int);
}

static int
setup_signal(int sig)
{
	struct sigaction sa;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler;
	sigemptyset(&sa.sa_mask);
	return sigaction(sig, &sa, NULL);
}

static void
signal_handler(int sig, siginfo_t *si, void *data)
{
	printf("Signal arrived: %d\n", sig);
	printf("\tUid: %u, Pid: %u\n", si->si_uid, si->si_pid);
	printf("\tValue: %d\n", si->si_value.sival_int);
}
