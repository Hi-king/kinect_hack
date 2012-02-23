#include <opencv2\opencv.hpp>

#include <gl\glut.h>
#include <gl\GL.h>


void disp(){
	glClearColor(1.0, 1.0, 1.0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void glut_keyboard(unsigned char key, int x, int y){
	if(key=='q'){
		exit(0);
	}
	
}

int main(int argc, char** argv){

	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(100, 100);
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );

	glutCreateWindow("test window");
	glutDisplayFunc(disp);
	glutKeyboardFunc(glut_keyboard);
	glutMainLoop();
	
	while(1){
		disp();		
	}
	
	return 0;

}