#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#if (__APPLE__ & __MACH__)
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif
#include <portaudio.h>
#ifdef WIN32
  #include <windows.h>
  #if PA_USE_ASIO
    #include <pa_asio.h>
  #endif
#endif

#include "common.h"
#include "aio.h"
#include "sigproc.h"

float rawmin, rawmax, rawavg, rawamp;
float ps_out[MAX_FRAMES_PER_BUFFER], psmin, psmax, psavg, psamp;
struct FFT_OUT fft_out[MAX_FRAMES_PER_BUFFER];
int omegaskip=1;
int triggered=0;
aio_t *aHdl;

/**************************** GL stuff ************************************/
void change_size(int w, int h) {
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D (0.0, (GLdouble)MAX_FRAMES_PER_BUFFER, 0.0, (GLdouble) h);
}
void process_normal_keys(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0);
    } else if(key == 'a') {
        if(omegaskip < 100)
            omegaskip++;
    } else if(key == 'z') {
        if(omegaskip > 1)
            omegaskip--;
    } else if(key == 't') {
        triggered = !triggered;
    }
    printf("omegaskip = %d triggered = %d\n", omegaskip, triggered);
}
void draw(void)
{
    int i, j, trig;
    float a, b;
    int index;
    float freq, t;
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0); 
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//    aio_acquire(aHdl);
    rawstat(aHdl->data, &rawmin, &rawmax, &rawavg);
    rawamp = MAX(rawmax,rawmin);
    fft(aHdl->data, fft_out);
    fft2ps(fft_out, ps_out, &psmin, &psmax, &psavg);
    psamp = psmax - psmin;

    freq = psmainfreq(aHdl->rate, ps_out, &index, &t);
    printf("%g %d %g\n", freq, index, t);

    glColor4f(1.0, 0,0, 0.0);
    glBegin(GL_LINE_STRIP);
    if(triggered) {
        j = 0;
        trig = 0;
        b = 100.0;
        for(i=0; i<MAX_FRAMES_PER_BUFFER; i++) {
            a = (aHdl->data[i]-rawavg)/rawamp*100.0;
            if(((a<10) || (a-b < 2)) && (trig==0)) {
                b = a;
                j++;
                continue;
            } else
                trig = 1;
            glVertex2f(i-j, 300+a);
        }
    } else {
        for(i=0; i<MAX_FRAMES_PER_BUFFER; i++)
            glVertex2f(i, 300+(aHdl->data[i]-rawavg)/rawamp*100.0);
    }
    glEnd();
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(i=0; i<MAX_FRAMES_PER_BUFFER/2; i++)
        glVertex2f(i*2, 1+(ps_out[i/omegaskip])/psamp*200.0);
    glEnd();

    glRasterPos2i(1, 187);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'F');
    glRasterPos2i(81, 187);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'F');
    glRasterPos2i(161, 187);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'T');

/*
    glPushMatrix();
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'A');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'T');
    glPopMatrix();
*/
    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    int aid=-1;
    double rate=-1.0;
    
    if(argc == 2) {
        if(argv[1][0] == '-') {
            if(argv[1][1] == 'l') {
                aio_list_devices();
                return EXIT_SUCCESS;
            }
        } else {
            aid = atoi(argv[1]);
        }
    } else if(argc > 2) {
        aid = atoi(argv[1]);
        rate = atof(argv[2]);
    }
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(10, 10);
    glutInitWindowSize(800, 400);
    glutCreateWindow("AudioTune");
    glutDisplayFunc(draw);
    glutIdleFunc(draw);
    glutReshapeFunc(change_size);
    glutKeyboardFunc(process_normal_keys);

    aHdl = aio_init(aid, rate);
    glutMainLoop();
    aio_close(aHdl);
    
    return EXIT_SUCCESS;
}
