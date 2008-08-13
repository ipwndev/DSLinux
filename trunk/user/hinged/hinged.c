/* Very simple daemon to handle hinge events on Nintendo DS
 * By Sonny_Jim
 *
 * SW_0 is triggered in /dev/input/event0 when the hinge is open/closed
*/
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>

#define HINGEDEV "/dev/input/event0"
#define EXECHINGEOPEN "/usr/bin/lidopen"
#define EXECHINGECLOSE "/usr/bin/lidclose"

int main(int argc, char **argv)
{
	int fd, rd, i, ret;
	struct input_event ev[64];
	
	// if argc = -h then print help
	if ((fd = open(HINGEDEV, O_RDONLY)) < 0) 
	{
	perror("hinged");
	return 1;
	}
	

	while (1) 
		{
		rd = read(fd, ev, sizeof(struct input_event) * 64);
		for (i = 0; i < rd / sizeof(struct input_event); i++)
			if (ev[i].type == EV_SW)
			{
				if (ev[i].value == 1)
				{
					// Do close cmd
					ret = system(EXECHINGECLOSE);
					// TODO: Check exit status of child
					printf("hinged: Hinge closed, Executed %s, Returned %d\n", EXECHINGECLOSE, ret);
				}
				else
				{
					// Do open cmd
					ret = system(EXECHINGEOPEN);	
					printf("Hinge opened, Executed %s, Returned %d\n", EXECHINGEOPEN, ret);
				}
			}
	
		}
}
