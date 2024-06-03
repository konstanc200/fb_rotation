#include <stdlib.h>
#include <fcntl.h>                                                            
#include <sys/mman.h>                                                         
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <stdio.h> 
#include <limits.h>
#include <math.h>

static FILE *fd2;                                   
                                                    
int rand_num_proc(int max) {                        
    int c;                                          
    do {                                            
        c = fgetc(fd2);                             
                                                    
    } while (c >= (UCHAR_MAX + 1 / max * max));     
                                                    
    return c % max;                                 
}
                                                   
                                                    
int rand_num_generator(void) {                                      
    int rand_num_arr[3], rand_num = 0;                
    int max = 3;                                      
    size_t size = 3;                                  
                                                      
    fd2 = fopen("/dev/urandom", "rb");                
    if (fd2 == NULL) {                                
        perror("**ERROR LOADING URANDOM DEVICE**");   
        return 1;                                     
                                                      
    }                                                 
                                                      
    rand_num_arr[0] = rand_num_proc(3);               
    if (rand_num_arr[0] < 2) {                        
        rand_num_arr[1] = rand_num_proc(9);           
        rand_num_arr[2] = rand_num_proc(9);                        
    }else {                                           
        rand_num_arr[1] = rand_num_proc(6);           
        rand_num_arr[2] = rand_num_proc(6);           
    }                                                 
                                                                  
    rand_num = rand_num*10 + rand_num_arr[0];         
    rand_num = rand_num*10 + rand_num_arr[1];         
    rand_num = rand_num*10 + rand_num_arr[2];         

    fclose(fd2);         
                         
    return rand_num;            
}                        


int main(int argc, char *argv[]) {
	
	int SCREEN_WIDTH, SCREEN_HEIGHT, BYTES_PER_PIXEL, fd, x, y;
	unsigned char * fbmap;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	fd = open("/dev/fb0", O_RDWR); // open fb device
	if (fd == -1) {
		perror("**CAN NOT OPEN Framebuffer DEVICE**");
		return 1;
	}
 
	ioctl(fd, FBIOGET_VSCREENINFO, &vinfo); // get the screen info
	ioctl(fd, FBIOGET_FSCREENINFO, &finfo); // get the screen info
	
	SCREEN_WIDTH = vinfo.xres;                                                    
	SCREEN_HEIGHT = vinfo.yres;
	BYTES_PER_PIXEL = vinfo.bits_per_pixel / 8; 

	fputs("\e[?251", stdout); // hide the cursor  

	fbmap = mmap(NULL, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	fbmap += SCREEN_WIDTH * BYTES_PER_PIXEL + BYTES_PER_PIXEL; // jump to the first line of the rectangl


	int blue, green, red;
	int blue_2, green_2, red_2;
	long int location = 0;
	int i = 0; 

	double xoffset;
	int x_start = vinfo.xres/2;
	int y_start = vinfo.yres/4;
	int y_size = 1000;
	int x_size = 10;
	
	double phi_deg; // degrees of the the angle phi which the inclined line makes with the vertical axis

	double phi_rad;
	double i_xoffset, tan_phi; // subtracted by xoffset at each x line iteration after the intial point drawen	


	while (i<1000) {

		blue = rand_num_generator();
		green = rand_num_generator();
		red = rand_num_generator();
		xoffset = 0;
		i_xoffset = 0;
		tan_phi = 0;
		phi_deg = 0;
		phi_rad = 0;

		for (int i_phi;i_phi <=360; i_phi++){
			phi_deg = i_phi;
			phi_rad = 0.0174532925 * phi_deg;
    		tan_phi = tan(phi_rad);

			double x_size_2 = 20;

			xoffset = y_size/2 * tan_phi;
        	i_xoffset = tan_phi;

			

		for (y = 0; y < vinfo.yres; y++) {

			if(y > y_start && y < y_start + y_size) { 
				xoffset = xoffset - i_xoffset;
			}

			for (x = 0; x < vinfo.xres; x++) { 
	

				location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
			
				if((y >= y_start && y < y_start + y_size) && ((x >= x_start + xoffset) && (x < ((x_start + xoffset) + x_size_2) ) )) { //				draw the rotated line
                	fbmap[location] = blue; // blue 
                	fbmap[location + 1] = green; // green
                	fbmap[location + 2] = red; // red
                	fbmap[location + 3] = 0; // transparency

				}
			}
		}
//		sleep(0.5);

		}

		sleep(1);
		i=i+10;	

	}
	munmap(fbmap, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
	close(fd);
	
	printf("\e[?25h"); // show cursor
	return 0;

}
