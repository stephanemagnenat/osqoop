#include <stdlib.h>
#include <stdio.h>
#include <usb.h>
#include <unistd.h>
#include <sys/types.h>

// for each fx2, load firmware on them
int main(int argc, char *argv[])
{
	struct usb_bus *busses;
	struct usb_bus *bus;
	struct usb_device *dev;
	
	if (argc != 2)
	{
		fprintf(stderr, "%s: error: firmware filname must be passed as argument\n", argv[0]);
		return 1;
	}
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
	busses = usb_get_busses();
	
	// iterator on all buses
	for (bus = busses; bus; bus = bus->next)
	{
		// iterate on all devices
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if ((dev->descriptor.idVendor == 0x04b4) && (dev->descriptor.idProduct == 0x8613))
			{
				char buffer[1024];
				uid_t uid = getuid();
				gid_t gid = getgid();
				
				system("modprobe -r usbtest");
				snprintf(buffer, sizeof(buffer), "fxload -t fx2 -I %s -D /proc/bus/usb/%s/%s", argv[1], bus->dirname, dev->filename);
				buffer[sizeof(buffer) - 1] = 0;
				system(buffer);
				snprintf(buffer, sizeof(buffer), "/dev/bus/usb/%s/%s", bus->dirname, dev->filename);
				if (chown(buffer, uid, gid) != 0)
				{
					perror("chown");
					return 2;
				}
				return 0;
			}
		}
	}
	return 2;
}
