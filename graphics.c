#include "common.h"
#include "graphics.h"

void changeSize(int w, int h) {
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D (0.0, (GLdouble) w, 0.0, (GLdouble) h);
}
void processNormalKeys(unsigned char key, int x, int y) {
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
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0); 
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    readonce();
    raw2float();
    fft(fft_in);
    fftraw2spec();
    
    glColor4f(1.0, 0,0, 0.0);
    glBegin(GL_LINE_STRIP);
    if(triggered) {
        j = 0;
        trig = 0;
        b = 100.0;
        for(i=0; i<800; i++) {
            a = (fft_in[i]-rawavg)/rawamp*100.0;
            if(((a<10) || (a-b < 2)) && (trig==0)) {
                b = a;
                j++;
                continue;
            } else
                trig = 1;
            glVertex2f(i-j, 300+a);
        }
    } else {
        for(i=0; i<800; i++)
            glVertex2f(i, 300+(fft_in[i*5]-rawavg)/rawamp*100.0);
    }
    glEnd();
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(i=0; i<800; i++)
        glVertex2f(i, 1+(fft_out[i/omegaskip]-fftavg)/fftamp*100.0);
    glEnd();
    glutSwapBuffers();
}


int main(int argc, char **argv)
{
    aio_t *aHdl;
    int i;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(10, 10);
    glutInitWindowSize(800, 400);
    glutCreateWindow("FFT");
    glutDisplayFunc(draw);
    glutIdleFunc(draw);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processNormalKeys);

    aHdl = aio_init(7, -1);
    glutMainLoop();
    aio_close(aHdl);

    return EXIT_SUCCESS;
}

