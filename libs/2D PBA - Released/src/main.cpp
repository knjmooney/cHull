/*
Author: Cao Thanh Tung
Date: 21/01/2010

File Name: main.cpp

===============================================================================

Copyright (c) 2010, School of Computing, National University of Singapore. 
All rights reserved.

Project homepage: http://www.comp.nus.edu.sg/~tants/pba.html

If you use PBA and you like it or have comments on its usefulness etc., we 
would love to hear from you at <tants@comp.nus.edu.sg>. You may share with us
your experience and any possibilities that we may improve the work/code.

===============================================================================

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the National University of University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission from the National University of Singapore. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <GL/glut.h>

#include "pba/pba2D.h"

//**********************************************************************************************
//* Input parameters
//**********************************************************************************************
int fboSize		= 512;
int nVertices   = 100; 

int phase1Band  = 16; 
int phase2Band  = 16; 
int phase3Band  = 16; 

//**********************************************************************************************
//* Global Variable
//**********************************************************************************************
int winSize = 512; 

typedef struct {
    double totalDistError, maxDistError; 
    int errorCount; 
} ErrorStatistics;

short *inputPoints, *inputVoronoi, *outputVoronoi; 
ErrorStatistics pba; 

GLuint texture; 
float scale = 1.0, transX = 0.0, transY = 0.0; 
bool isLeftMouseActive = false, isRightMouseActive = false; 
int oldMouseX = 0, oldMouseY = 0; 

//**********************************************************************************************
//* Random Point Generator
//**********************************************************************************************
// Random number generator, obtained from http://oldmill.uchicago.edu/~wilder/Code/random/
unsigned long z, w, jsr, jcong; // Seeds
void randinit(unsigned long x_) 
{ z =x_; w = x_; jsr = x_; jcong = x_; }
unsigned long znew() 
{ return (z = 36969 * (z & 0xfffful) + (z >> 16)); }
unsigned long wnew() 
{ return (w = 18000 * (w & 0xfffful) + (w >> 16)); }
unsigned long MWC()  
{ return ((znew() << 16) + wnew()); }
unsigned long SHR3()
{ jsr ^= (jsr << 17); jsr ^= (jsr >> 13); return (jsr ^= (jsr << 5)); }
unsigned long CONG() 
{ return (jcong = 69069 * jcong + 1234567); }
unsigned long rand_int()         // [0,2^32-1]
{ return ((MWC() ^ CONG()) + SHR3()); }
// double random()     // [0,1)
// { return ((double) rand_int() / (double(ULONG_MAX)+1)); }

// Generate input points
void generateRandomPoints(int width, int height, int nPoints)
{	
    int tx, ty; 

    randinit(0);

    for (int i = 0; i < width * height * 2; i++)
        inputVoronoi[i] = MARKER; 

    for (int i = 0; i < nPoints; i++)
    {
        do { 
            tx = int(random() * width); 
            ty = int(random() * height); 
        } while (inputVoronoi[(ty * width + tx) * 2] != MARKER); 

        inputVoronoi[(ty * width + tx) * 2    ] = tx; 
        inputVoronoi[(ty * width + tx) * 2 + 1] = ty; 

        inputPoints[i * 2    ] = tx; 
        inputPoints[i * 2 + 1] = ty; 
    }	
}

/**********************************************************************************************
* Deinitialization
**********************************************************************************************/
void deinitialization()
{
    pba2DDeinitialization(); 

    free(inputPoints); 
    free(inputVoronoi); 
    free(outputVoronoi); 
}

/**********************************************************************************************
* Initialization                                                                           
**********************************************************************************************/
void initialization()
{
    pba2DInitialization(fboSize); 

    inputPoints     = (short *) malloc(nVertices * 2 * sizeof(short)); 
    inputVoronoi    = (short *) malloc(fboSize * fboSize * 2 * sizeof(short)); 
    outputVoronoi   = (short *) malloc(fboSize * fboSize * 2 * sizeof(short)); 
}

/**********************************************************************************************
* Verify the output Voronoi Diagram
**********************************************************************************************/
void verifyResult(ErrorStatistics *e) 
{
    e->totalDistError = 0.0; 
    e->maxDistError = 0.0; 
    e->errorCount = 0; 

    int tx, ty; 
    double dist, myDist, correctDist, error;

    for (int i = 0; i < fboSize; i++)
        for (int j = 0; j < fboSize; j++) {
            int id = j * fboSize + i; 

            tx = outputVoronoi[id * 2] - i; 
            ty = outputVoronoi[id * 2 + 1] - j; 
            correctDist = myDist = tx * tx + ty * ty; 

            for (int t = 0; t < nVertices; t++) {
                tx = inputPoints[t * 2] - i; 
                ty = inputPoints[t * 2 + 1] - j; 
                dist = tx * tx + ty * ty; 

                if (dist < correctDist)
                    correctDist = dist; 
            }

            if (correctDist != myDist) {
                error = fabs(sqrt(myDist) - sqrt(correctDist)); 

                e->errorCount++; 
                e->totalDistError += error; 

                if (error > e->maxDistError)
                    e->maxDistError = error; 
            }
        }
}

void printStatistics(ErrorStatistics *e)
{
    double avgDistError = e->totalDistError / e->errorCount; 

    if (e->errorCount == 0)
        avgDistError = 0.0; 

    printf("* Error count           : %i -> %.3f%\n", e->errorCount, 
        (double(e->errorCount) / nVertices) * 100.0);
    printf("* Max distance error    : %.5f\n", e->maxDistError);
    printf("* Average distance error: %.5f\n", avgDistError);
}

/**********************************************************************************************
* Run the tests
**********************************************************************************************/
void runTests()
{
    printf("Generate input...\n"); 
    generateRandomPoints(fboSize, fboSize, nVertices); 

    printf("Running PBA to compute 2D Voronoi Diagram...\n"); 
    pba2DVoronoiDiagram(inputVoronoi, outputVoronoi, phase1Band, phase2Band, phase3Band); 

    printf("Verifying the result...\n"); 
    verifyResult(&pba);

    printf("-----------------\n");
    printf("Texture: %dx%d\n", fboSize, fboSize);
    printf("Points: %d\n", nVertices);
    printf("-----------------\n");

    printStatistics(&pba); 
}

void glut_mouse(int button, int state, int x, int y) 
{
    if (state == GLUT_UP)
        switch (button)
    {
        case GLUT_LEFT_BUTTON:
            isLeftMouseActive = false;
            break;
        case GLUT_RIGHT_BUTTON:
            isRightMouseActive = false; 
            break; 
    }

    if (state == GLUT_DOWN)
    {
        oldMouseX = x;
        oldMouseY = y;

        switch (button)
        {
        case GLUT_LEFT_BUTTON:
            isLeftMouseActive = true; 
            break;
        case GLUT_RIGHT_BUTTON:
            isRightMouseActive = true;
            break;
        }
    }		
}

void glut_mouseMotion(int x, int y) 
{
    if (isLeftMouseActive) {
        transX += 2.0 * double(x - oldMouseX) / scale / fboSize; 
        transY -= 2.0 * double(y - oldMouseY) / scale / fboSize; 
        glutPostRedisplay(); 
    }
    else if (isRightMouseActive) {
        scale -= (y - oldMouseY) * scale / 400.0;
        glutPostRedisplay(); 
    } 

    oldMouseX = x; oldMouseY = y; 
}

#define BYTE unsigned char

void generateTexture() 
{
    // Generate the color map
    BYTE *colorMap = (BYTE *) malloc(fboSize * fboSize * 2 * sizeof(BYTE)); 

    for (int i = 0; i < nVertices; i++) {
        BYTE g = int(128 * random()); 
        BYTE b = int(128 * random()); 

        int id = inputPoints[i * 2 + 1] * fboSize + inputPoints[i * 2]; 

        colorMap[id * 2 + 0] = g; 
        colorMap[id * 2 + 1] = b; 
    }

    // Generate the voronoi diagram color 
    BYTE *voronoi = (BYTE *) malloc(fboSize * fboSize * 4 * sizeof(BYTE)); 

    int tx, ty; 

    // Generate texture
    for (int i = 0; i < fboSize; i++)
        for (int j = 0; j < fboSize; j++) {
            int id = j * fboSize + i; 

            tx = outputVoronoi[id * 2]; 
            ty = outputVoronoi[id * 2 + 1]; 

            int vid = ty * fboSize + tx; 
            voronoi[id * 4 + 0] = 0; 
            voronoi[id * 4 + 1] = colorMap[vid * 2 + 0]; 
            voronoi[id * 4 + 2] = colorMap[vid * 2 + 1]; 
            voronoi[id * 4 + 3] = 255; 
        }

    // Create a texture
    glGenTextures(1, &texture); 
    glBindTexture(GL_TEXTURE_2D, texture); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboSize, fboSize, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, voronoi); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    free(voronoi); 
    free(colorMap); 
}

void drawPoints() 
{
    glColor3d(1.0, 1.0, 1.0); 
    glPointSize(3.0);

    glBegin(GL_POINTS); 

    for (int i = 0; i < nVertices; i++)
        glVertex2d(double(inputPoints[i * 2]) / fboSize, 
        double(inputPoints[i * 2 + 1]) / fboSize); 

    glEnd(); 
}

void glutDisplay() {
    // Initialization
    glViewport(0, 0, (GLsizei) winSize, (GLsizei) winSize); 

    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT); 

    // Setup projection matrix
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); 
    gluOrtho2D(0.0, 1.0, 0.0, 1.0); 

    // Setup modelview matrix
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity(); 
    glScalef(scale, scale, 1.0);
    glTranslatef(transX, transY, 0.0);

    // Setting up lights
    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST); 
    glEnable(GL_TEXTURE_2D); 

    // Draw the sample scene
    glBindTexture(GL_TEXTURE_2D, texture); 

    glColor3d(1.0, 1.0, 1.0); 

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0); 
    glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0); 
    glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0); 
    glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0); 		
    glEnd(); 

    glBindTexture(GL_TEXTURE_2D, 0); 
    glDisable(GL_TEXTURE_2D); 

    drawPoints(); 

    glutSwapBuffers();     
}

int main(int argc,char **argv)
{
    glutInitWindowPosition(0, 0); 
    glutInitWindowSize(winSize, winSize); 
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA); 
    glutInit(&argc, argv); 
    glutCreateWindow("2D PBA"); 

    initialization(); 
    runTests();

    generateTexture(); 

    glutDisplayFunc(glutDisplay); 
    glutMouseFunc(glut_mouse); 
    glutMotionFunc(glut_mouseMotion); 
    glutMainLoop();  

    deinitialization(); 

    return 0;
}
