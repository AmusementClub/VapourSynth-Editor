#include <string.h>

extern int job_watcher_main(int argc, char *argv[]);
extern int job_server_main(int argc, char *argv[]);
extern int vsedit_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[1], "--job-server") == 0)
			return job_server_main(argc, argv);
		if (strcmp(argv[1], "--job-watcher") == 0)
			return job_watcher_main(argc, argv);
	}
	return vsedit_main(argc, argv);
}
