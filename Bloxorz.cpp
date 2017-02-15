#include <ao/ao.h>
#include <mpg123.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define BITS 8
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true, mouse_hit;

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS) {
              mouse_hit=1;
            }
            if (action == GLFW_RELEASE) {
              triangle_rot_dir *= -1;
              mouse_hit=0;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    //glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createSwitch (VAO** line_1, VAO** line_2, float width, float length, float height)
{
  float reduce=0.04;
  height+=0.0009;
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data_1 [] = {
    width-reduce,length,height, // vertex 1
    width,length-reduce,height, // vertex 2
    -width+reduce,-length,height, // vertex 3

    -width+reduce, -length,height, // vertex 3
    -width, -length+reduce,height, // vertex 4
    width-reduce,length,height, // vertex 1
  };

  static const GLfloat color_buffer_data_1 [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };

  static const GLfloat vertex_buffer_data_2 [] = {
    -width+reduce,length,height, // vertex 1
    -width,length-reduce,height, // vertex 2
    width-reduce,-length,height, // vertex 3

    width-reduce, -length,height, // vertex 3
    width, -width+reduce,height, // vertex 4
    -width+reduce,length,height, // vertex 1
  };

  static const GLfloat color_buffer_data_2 [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  *line_1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_1, color_buffer_data_1, GL_FILL);
  *line_2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_2, color_buffer_data_2, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float currX, currY, TIME_X, TIME_Y, TIME_Z, SCORE_X, SCORE_Y, SCORE_Z;
int currIndexX, currIndexY;

class CuboidColor {
  public:
    float face[6][3];
};

void createCuboid(float length, float width, float height, VAO** cuboid, bool block, CuboidColor color, bool is_fragile, bool is_bridge) {

  GLfloat vertex_buffer_data [] = {

    // face 1
    width,length,height, // vertex 1
    width,-length,height, // vertex 2
    -width,-length,height, // vertex 3
    -width,-length,height, // vertex 3
    -width, length,height, // vertex 4
    width,length,height, // vertex 1

    // face 2
    width,length,-height, // vertex 5
    width,-length,-height, // vertex 6
    -width,-length,-height, // vertex 7
    -width,-length,-height, // vertex 7
    -width, length,-height, // vertex 8
    width,length,-height, // vertex 5

    // face 3
    width,length,height, // vertex 1
    width,length,-height, // vertex 5
    width,-length,height, // vertex 2

    width,length,-height, // vertex 5
    width,-length,height, // vertex 2
    width,-length,-height, // vertex 6

    // face 4
    -width, length,height, // vertex 4
    -width, length,-height, // vertex 8
    -width,-length,height, // vertex 3

    -width, length,-height, // vertex 8
    -width,-length,height, // vertex 3
    -width,-length,-height, // vertex 7

    // face 5
    width,length,height, // vertex 1
    -width, length,height, // vertex 4
    width,length,-height, // vertex 5

    -width, length,height, // vertex 4
    width,length,-height, // vertex 5
    -width, length,-height, // vertex 8

    // face 6
    -width,-length,height, // vertex 3
    width,-length,-height, // vertex 6
    -width, length,height, // vertex 4

    width,-length,-height, // vertex 6
    -width, length,height, // vertex 4
    -width,-length,-height, // vertex 7
  };

  int i;
  if(!block) {
    GLfloat color_buffer_data [] = {
      
      // face 1 (top-face)
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,

      // face 2 (top-face)
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,

      // face 3
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,
      // face 4
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,
      // face 5
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,
      // face 6
      0,0,0,
      0.2,is_fragile*0.2,is_bridge*0.2,
      0.4,is_fragile*0.4,is_bridge*0.4,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.6,is_fragile*0.6,is_bridge*0.6,
      0.8,is_fragile*0.8,is_bridge*0.8,
    };
    *cuboid = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
  }
  else {
    GLfloat color_buffer_data [] = {
      // face 1 (top-face)
      color.face[0][0],color.face[0][1],color.face[0][2],
      color.face[0][0],color.face[0][1],color.face[0][2],
      color.face[0][0],color.face[0][1],color.face[0][2],
      color.face[0][0],color.face[0][1],color.face[0][2],
      color.face[0][0],color.face[0][1],color.face[0][2],
      color.face[0][0],color.face[0][1],color.face[0][2],
      // face 2 (top-face)
      color.face[1][0],color.face[1][1],color.face[1][2],
      color.face[1][0],color.face[1][1],color.face[1][2],
      color.face[1][0],color.face[1][1],color.face[1][2],
      color.face[1][0],color.face[1][1],color.face[1][2],
      color.face[1][0],color.face[1][1],color.face[1][2],
      color.face[1][0],color.face[1][1],color.face[1][2],
      // face 3
      color.face[2][0],color.face[2][1],color.face[2][2],
      color.face[2][0],color.face[2][1],color.face[2][2],
      color.face[2][0],color.face[2][1],color.face[2][2],
      color.face[2][0],color.face[2][1],color.face[2][2],
      color.face[2][0],color.face[2][1],color.face[2][2],
      color.face[2][0],color.face[2][1],color.face[2][2],
      // face 4
      color.face[3][0],color.face[3][1],color.face[3][2],
      color.face[3][0],color.face[3][1],color.face[3][2],
      color.face[3][0],color.face[3][1],color.face[3][2],
      color.face[3][0],color.face[3][1],color.face[3][2],
      color.face[3][0],color.face[3][1],color.face[3][2],
      color.face[3][0],color.face[3][1],color.face[3][2],
      // face 5
      color.face[4][0],color.face[4][1],color.face[4][2],
      color.face[4][0],color.face[4][1],color.face[4][2],
      color.face[4][0],color.face[4][1],color.face[4][2],
      color.face[4][0],color.face[4][1],color.face[4][2],
      color.face[4][0],color.face[4][1],color.face[4][2],
      color.face[4][0],color.face[4][1],color.face[4][2],
      // face 6
      color.face[5][0],color.face[5][1],color.face[5][2],
      color.face[5][0],color.face[5][1],color.face[5][2],
      color.face[5][0],color.face[5][1],color.face[5][2],
      color.face[5][0],color.face[5][1],color.face[5][2],
      color.face[5][0],color.face[5][1],color.face[5][2],
      color.face[5][0],color.face[5][1],color.face[5][2],
    };
    *cuboid = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
  }

  // create3DObject creates and returns a handle to a VAO that can be used later
}

int total_time, total_score, DYING;

double mouse_xpos, mouse_ypos;

float currAxis[3], DYING_inc, DYING_rot, prev_mouse_x, prev_mouse_y;

bool change_level, Y_NEG, Y_POS, X_NEG, X_POS;

float target_mouse_x, target_mouse_y;

double mouse_xoffset, mouse_yoffset;

float getMouseCoordX () {
  return (((mouse_xpos+4)/800*8)-4);
}

float getMouseCoordY () {
  return (((600-mouse_ypos+4)/600*8)-4);
}

float changeMouseInX () {
  float diff=(getMouseCoordX()-prev_mouse_x);
  prev_mouse_x=getMouseCoordX();
  return(diff);
}

float changeMouseInY () {
  float diff=(getMouseCoordY()-prev_mouse_y);
  prev_mouse_y=getMouseCoordY();
  return(diff);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  mouse_xoffset=xoffset;
  mouse_yoffset=yoffset;
}

string getAxis() {
  if(currAxis[0]==1.0)
    return "x";
  else
    return "y";
}

void changeAxis() {
  if(currAxis[0]==1) {
    currAxis[0]=0;
    currAxis[1]=1;
  }
  else {
    currAxis[0]=1;
    currAxis[1]=0;
  }
}

void playmusic() {
    
}

class SevenSegment {
   public:
      float x;
      float y;
      float y_shift;
      float x_shift;
      float length;
      float width;
      VAO *a, *b, *c, *d, *e, *f, *g;
      bool A, B, C, D, E, F, G;

      void create (float X_SHIFT, float Y_SHIFT, int number) {

        this->x_shift = X_SHIFT;
        this->y_shift = Y_SHIFT;
        this->A=1; this->B=1; this->C=1; this->D=1; this->E=1; this->F=1; this->G=1;

        float x_coord, y_coord, red=0.5, blue=0, green=0, x_shift, y_shift;

        if(number > 10 || number < 0 ) 
          return;

        switch(number){
          case 0:
            this->G=0;
            break; //optional
          case 1:
            this->G=0;
            this->F=0;
            this->E=0;
            this->A=0;
            this->D=0;
            break; //optional
          case 2:
            this->F=0;
            this->C=0;
            break; //optional
          case 3:
            this->F=0;
            this->E=0;
            break; //optional
          case 4:
            this->A=0;
            this->E=0;
            this->D=0;
            break; //optional
          case 5:
            this->B=0;
            this->E=0;
            break; //optional
          case 6:
            this->B=0;
            break; //optional
          case 7:
            this->G=0;
            this->E=0;
            this->F=0;
            this->D=0;
            break; //optional
          case 8:
            break; //optional
          case 9:
            this->E=0;
            break; //optional
        }
        
        if(this->G) {
          y_shift = 0, x_coord=0.09, y_coord=0.04, x_shift=0;

          GLfloat vertex_buffer_data_g [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_g [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          G = 1;
          this->g = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_g, color_buffer_data_g, GL_FILL);
        }

        if(this->A) {
          y_shift = 0.25, x_coord=0.09, y_coord=0.04, x_shift=0;

          GLfloat vertex_buffer_data_a [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_a [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          A = 1;
          this->a = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_a, color_buffer_data_a, GL_FILL);
        }

        if(this->D) {
          x_coord=0.09, y_coord=0.04, x_shift=0, y_shift = -0.25;

          GLfloat vertex_buffer_data_d [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_d [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          D = 1;
          this->d = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_d, color_buffer_data_d, GL_FILL);
        }

        if(this->C) {
          x_shift = 0.11, x_coord=0.025, y_coord=0.12, y_shift=-0.12;

          GLfloat vertex_buffer_data_c [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_c [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          C = 1;
          this->c = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_c, color_buffer_data_c, GL_FILL);
        }

        
        if(this->B) {
          x_shift = 0.11, x_coord=0.025, y_coord=0.12, y_shift=0.12;

          GLfloat vertex_buffer_data_b [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_b [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          B = 1;
          this->b = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_b, color_buffer_data_b, GL_FILL);
        }

        if(this->E) {
          x_shift = -0.11, x_coord=0.025, y_coord=0.12, y_shift=-0.12;

          GLfloat vertex_buffer_data_e [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_e [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          E = 1;
          this->e = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_e, color_buffer_data_e, GL_FILL);
        }

        if(this->F) {
          x_shift = -0.11, x_coord=0.025, y_coord=0.12, y_shift=0.12;

          GLfloat vertex_buffer_data_f [] = {
            -x_coord+x_shift,-y_coord+y_shift,0, // vertex 1
            -x_coord+x_shift,y_coord+y_shift,0, // vertex 2
            x_coord+x_shift,y_coord+y_shift,0, // vertex 3

            x_coord+x_shift,y_coord+y_shift,0, // vertex 3
            x_coord+x_shift,-y_coord+y_shift,0, // vertex 4
            -x_coord+x_shift,-y_coord+y_shift,0  // vertex 1
          };

          GLfloat color_buffer_data_f [] = {
            red,green,blue, // color 1
            red,green,blue, // color 2
            red,green,blue, // color 3

            red,green,blue, // color 3
            red,green,blue, // color 4
            red,green,blue  // color 1
          };

          F = 1;
          this->f = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_f, color_buffer_data_f, GL_FILL);
        }
      }
};

SevenSegment score_board[4];

void updateScore (float x, float y) {
  if((total_score/100)==0) {
    score_board[0].create(x, y, total_score%10);
    score_board[1].create(x-0.3, y, total_score/10);
  }
}

class Tiles {
  public:
    VAO* body;
    VAO* line_1;
    VAO* line_2;
    float width;
    float height;
    float length;
    float x;
    float y;
    bool status;
    bool is_switch;
    bool is_bridge;
    bool is_fragile;
    bool is_finish;
    bool toggle_swtich;

    void create(bool is_switch, bool is_fragile, bool is_bridge) {

      int i;
      CuboidColor block_color;

      if(is_bridge&&is_fragile) {
        this->is_finish=1;
        this->is_fragile=0;
        this->is_bridge=0;
      }
      else {
        this->is_finish=0;
        this->is_fragile=is_fragile;
        this->is_bridge=is_bridge;
      }

      this->width=0.4;
      this->length=0.4;
      this->height=0.2;
      this->status=1;
      this->is_switch=is_switch;
      this->toggle_swtich=0;

      createCuboid(this->length/2, this->width/2, this->height/2, &this->body, 0, block_color, is_fragile, is_bridge);
      if(this->is_switch)
        createSwitch(&line_1, &line_2, this->width/2, this->length/2, this->height/2);

    }
};

class Block {
  public:
    VAO* body;
    float width;
    float height;
    float length;
    float x;
    float y;
    float z;
    float rotate_angle_y;
    float rotate_angle_x;
    float temp_rotate_x;
    float temp_rotate_y;
    bool status;
    bool standing;
    bool rotate_status;
    bool left;
    bool right;
    bool up;
    bool down;
    string name;
    glm::mat4 tempTranslate;
    glm::mat4 invTempTranslate;

    void create(float width, float length, float height, string name) {

      CuboidColor block_color;
      int i;

      this->name=name;
      this->width=width;
      this->length=length;
      this->height=height;
      if(name == "z")
        this->status=1;
      else
        this->status=0;
      this->standing=1;
      this->rotate_angle_y=0;
      this->rotate_angle_x=0;
      this->x=0;
      this->y=0;
      this->z=0;
      this->right=0;
      this->left=0;
      this->up=0;
      this->down=0;
      this->rotate_status=0;
      this->temp_rotate_x=0;
      this->temp_rotate_y=0;
      this->tempTranslate = glm::translate (glm::vec3(0, 0, this->height/2));
      this->invTempTranslate = glm::translate (glm::vec3(0, 0, 0.1));

      for(i=0;i<6;i++) {
        block_color.face[i][0]=0;
        block_color.face[i][1]=0.3;
        block_color.face[i][2]=1;
      }

      if(name == "z") {
        block_color.face[0][0]=0.5;
        block_color.face[0][1]=0.5;
        block_color.face[0][2]=0;
        block_color.face[1][0]=0.5;
        block_color.face[1][1]=0.5;
        block_color.face[1][2]=0;
      }
      else if(name == "x") {
        block_color.face[2][0]=0.5;
        block_color.face[2][1]=0.5;
        block_color.face[2][2]=0;
        block_color.face[3][0]=0.5;
        block_color.face[3][1]=0.5;
        block_color.face[3][2]=0;
      }
      else {
        block_color.face[4][0]=0.5;
        block_color.face[4][1]=0.5;
        block_color.face[4][2]=0;
        block_color.face[5][0]=0.5;
        block_color.face[5][1]=0.5;
        block_color.face[5][2]=0;
      }

      createCuboid(this->length/2, this->width/2, this->height/2, &this->body, 1, block_color, 0, 0);

    }

    void revolve_block(string move) {
      if(!this->rotate_status) {
        total_score++;
        updateScore(-3, 0);
        if(this->standing)
          this->standing=0;
        else
          this->standing=1;
        if(move == "left") {
          this->left=1;
          this->tempTranslate=glm::translate (glm::vec3(this->width/2, 0, this->height/2));
          this->invTempTranslate=glm::translate (glm::vec3(-this->width/2, 0, 0.1));
        }
        else if(move == "right") {
          this->right=1;
          this->tempTranslate=glm::translate (glm::vec3(-this->width/2, 0, this->height/2));
          this->invTempTranslate=glm::translate (glm::vec3(this->width/2, 0, 0.1));
        }
        else if(move == "up") {
          this->up=1;
          this->tempTranslate=glm::translate (glm::vec3(0, -this->length/2, this->height/2));
          this->invTempTranslate=glm::translate (glm::vec3(0, this->length/2, 0.1));
        }
        else {
          this->down=1;
          this->tempTranslate=glm::translate (glm::vec3(0, this->length/2, this->height/2));
          this->invTempTranslate=glm::translate (glm::vec3(0, -this->length/2, 0.1));
        }
        //changeAxis();
        this->rotate_status=1;
      }

    }
};

Tiles tiles[10][10];

Block block[3];

bool top_view, tower_view, level_view, block_view, front_view;

int currLevel;

void getCurrIndex() {
  int i, j;
  for(i=0;i<3;i++) {
    if(block[i].name == "z"&&block[i].status==1) {
      currIndexX=currX/0.4;
      currIndexY=currY/0.4;
    }
  }
}

void rotate_block() {
  int i;
  for(i=0;i<3;i++) {
    if(block[i].rotate_status) {
      if(block[i].left) {
        block[i].rotate_angle_y-=3;
        if(block[i].rotate_angle_y<=-91) {
          Y_NEG=1;
          Y_POS=0;
          X_POS=0;
          X_NEG=0;
          block[i].rotate_status=0;
          block[i].left=0;
          block[i].rotate_angle_y=0;
          block[i].tempTranslate = glm::translate (glm::vec3(0, 0, block[i].height/2));
          block[i].invTempTranslate=glm::translate (glm::vec3(0, 0, 0.1));
          if(block[i].name == "z") {
            block[0].status=0;
            block[2].status=1;
            block[2].x=block[0].x-0.4*1.5;
            block[2].y=block[0].y;
            currX=block[2].x;
            currY=block[2].y;
          }
          else if(block[i].name == "y") {
            block[1].x-=0.4;
            currX=block[1].x;
            currY=block[1].y;
          }
          else {
            block[2].status=0;
            block[0].status=1;
            block[0].x=block[2].x-0.4*1.5;
            block[0].y=block[2].y;
            currX=block[0].x;
            currY=block[0].y;
          }
        }
      }
      else if(block[i].right) {
        block[i].rotate_angle_y+=3;
        if(block[i].rotate_angle_y>=91) {
          Y_POS=1;
          Y_NEG=0;
          X_NEG=0;
          X_POS=0;
          block[i].rotate_status=0;
          block[i].right=0;
          block[i].rotate_angle_y=0;
          block[i].tempTranslate = glm::translate (glm::vec3(0, 0, block[i].height/2));
          block[i].invTempTranslate=glm::translate (glm::vec3(0, 0, 0.1));
          if(block[i].name == "z") {
            block[0].status=0;
            block[2].status=1;
            block[2].x=block[0].x+0.4*1.5;
            block[2].y=block[0].y;
            currX=block[2].x;
            currY=block[2].y;
          }
          else if(block[i].name == "y") {
            block[1].x+=0.4;
            currX=block[1].x;
            currY=block[1].y;
          }
          else {
            block[2].status=0;
            block[0].status=1;
            block[0].x=block[2].x+0.4*1.5;
            block[0].y=block[2].y;
            currX=block[0].x;
            currY=block[0].y;
          }
        }
      }
      else if(block[i].up) {
        block[i].rotate_angle_x-=3;
        if(block[i].rotate_angle_x<=-91) {
          X_NEG=1;
          X_POS=0;
          Y_POS=0;
          Y_NEG=0;
          block[i].rotate_status=0;
          block[i].up=0;
          block[i].rotate_angle_x=0;
          block[i].tempTranslate = glm::translate (glm::vec3(0, 0, block[i].height/2));
          block[i].invTempTranslate=glm::translate (glm::vec3(0, 0, 0.1));
          if(block[i].name == "z") {
            block[0].status=0;
            block[1].status=1;
            block[1].x=block[0].x;
            block[1].y=block[0].y+0.4*1.5;
            currX=block[1].x;
            currY=block[1].y;
          }
          else if(block[i].name == "x") {
            block[2].y+=0.4;
            currX=block[2].x;
            currY=block[2].y;
          }
          else {
            block[1].status=0;
            block[0].status=1;
            block[0].x=block[1].x;
            block[0].y=block[1].y+0.4*1.5;
            currX=block[0].x;
            currY=block[0].y;
          }
        }
      }
      else if(block[i].down) {
        block[i].rotate_angle_x+=3;
        if(block[i].rotate_angle_x>=91) {
          X_POS=1;
          X_NEG=0;
          Y_POS=0;
          Y_NEG=0;
          block[i].rotate_status=0;
          block[i].down=0;
          block[i].rotate_angle_x=0;
          block[i].tempTranslate = glm::translate (glm::vec3(0, 0, block[i].height/2));
          block[i].invTempTranslate=glm::translate (glm::vec3(0, 0, 0.1));
          if(block[i].name == "z") {
            block[0].status=0;
            block[1].status=1;
            block[1].x=block[0].x;
            block[1].y=block[0].y-0.4*1.5;
            currX=block[1].x;
            currY=block[1].y;
          }
          else if(block[i].name == "x") {
            block[2].y-=0.4;
            currX=block[2].x;
            currY=block[2].y;
          }
          else {
            block[1].status=0;
            block[0].status=1;
            block[0].x=block[1].x;
            block[0].y=block[1].y-0.4*1.5;
            currX=block[0].x;
            currY=block[0].y;
          }
        }
      }
    }
  }
}

bool isOnTile(int i, int j) {
  if(abs(currX-tiles[i][j].x)<0.1&&abs(currY-tiles[i][j].y)<0.1) {
    return 1;
  }
  else
    return 0;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, bool draw_screen)
{
  GLfloat fov = M_PI/2;

  camera_rotation_angle=90;

  glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);
  // Eye - Location of camera. Don't change unless you are sure!!
  // glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f)-1, -1, 5*sin(camera_rotation_angle*M_PI/180.0f-1) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!

    //glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(draw_screen) {

    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    int window_width=800, window_height=(0.8*600);

    glfwGetFramebufferSize(window, &window_width, &window_height);

    glViewport (0, 0, (GLsizei) 800, (GLsizei) (0.8*600));

    // Perspective projection for 3D views
    Matrices.projection = glm::perspective (fov, (GLfloat) (800) / (GLfloat) (0.8*600), 0.1f, 500.0f);

    float mouse_change_x, mouse_change_y;

    if(mouse_hit) {
      top_view=1;
      tower_view=0;
      level_view=0;
      block_view=0;
      front_view=0;
      mouse_change_x=changeMouseInX();
      mouse_change_y=changeMouseInY();
      target_mouse_x+=mouse_change_x;
      target_mouse_y+=mouse_change_y;
    }

    if(top_view) {
      glm::vec3 eye(0.4*5, 0.4*5, 5+mouse_yoffset);
      glm::vec3 up (0, 1, 0);
      glm::vec3 target (target_mouse_x, target_mouse_y, 0);
      Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    }
    else if(tower_view) {
      glm::vec3 eye(-1, -1, 4);
      glm::vec3 up (1, 1, 0);
      glm::vec3 target (0.4*5, 0.4*5, 0);
      Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    }
    else if(level_view) {
      glm::vec3 eye(0.4*5, -0.4*5, 2);
      glm::vec3 up (0, 1, 0);
      glm::vec3 target (0.4*5, 0.4*5, 0);
      Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    }
    else if(block_view) {
      if(getAxis() == "x") {
        glm::vec3 eye(currX-2, currY, 4);
        glm::vec3 up (0, 0, 1);
        glm::vec3 target (currX+0.4*10, currY, 1);
        Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      }
      else {
        glm::vec3 eye(currX, currY-2, 4);
        glm::vec3 up (0, 0, 1);
        glm::vec3 target (currX, currY+0.4*10, 1);
        Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      }
    }
    else if(front_view){
      if(getAxis() == "x") {
        glm::vec3 eye(currX+0.4, currY, 2);
        glm::vec3 up (0, 0, 1);
        glm::vec3 target (currX+0.4*5, currY, 0);
        Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      }
      else {
        glm::vec3 eye(currX, currY+0.4, 2);
        glm::vec3 up (0, 0, 1);
        glm::vec3 target (currX, currY+0.4*5, 0);
        Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      }
    }

    // Compute Camera matrix (view)
    //Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    //Matrices.view = glm::lookAt(glm::vec3(-1,-1,1), glm::vec3(0,0,0), glm::vec3(1,1,2)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our tr\ansformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;  // MVP = Projection * View * Model

    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */
    int i, j;

    for(i=0;i<3;i++) {
      if(block[i].status) {
        Matrices.model = glm::mat4(1.0f);
        if(DYING) {
          if(Y_NEG) {
            block[i].rotate_angle_y -= DYING_rot;
          }
          else if(X_POS) {
            block[i].rotate_angle_x += DYING_rot;
          }
          else if(Y_POS) {
            block[i].rotate_angle_y += DYING_rot;
          }
          else if(X_NEG) {
            block[i].rotate_angle_x -= DYING_rot;
          }
        }
        glm::mat4 translateBlock = glm::translate (glm::vec3(block[i].x, block[i].y, block[i].z-DYING_inc));        // glTranslatef
        glm::mat4 rotateBlockX = glm::rotate((float)(block[i].rotate_angle_x*M_PI/180.0f), glm::vec3(1,0,0));
        glm::mat4 rotateBlockY = glm::rotate((float)(block[i].rotate_angle_y*M_PI/180.0f), glm::vec3(0,1,0));
        Matrices.model *= (block[i].invTempTranslate*translateBlock*rotateBlockX*rotateBlockY*block[i].tempTranslate);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(block[i].body);
      }
    }

    rotate_block();

    if(DYING != 0) {
      DYING_inc+=0.08;
    }

    glm::mat4 translateTile;
      
    for(i=0;i<10;i++) {
      for(j=0;j<10;j++) {
        if(tiles[i][j].status) {
          Matrices.model = glm::mat4(1.0f);
          translateTile = glm::translate (glm::vec3(tiles[i][j].x, tiles[i][j].y, 0));        // glTranslatef
          Matrices.model *= (translateTile);
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(tiles[i][j].body);

          if(tiles[i][j].is_switch) {
            draw3DObject(tiles[i][j].line_1);
            draw3DObject(tiles[i][j].line_2);
          }
        }
      }
    }
  }
  else {

    //glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
    //glClearDepth (1.0f);

    // clear the color and depth in the frame buffer
    // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing

    int window_width, window_height;

    glfwGetFramebufferSize(window, &window_width, &window_height);
    // sets the viewport of openGL renderer
    glViewport (0, (int)(0.8*window_height), (int)(window_width), (int)(0.2*window_height));

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -0.7f, 0.7f, 0.1f, 500.0f);

    // Compute Camera matrix (view)
    //Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our tr\ansformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;  // MVP = Projection * View * Model

    glUseProgram (programID);

    int i;

    for(i=0;i<4;i++) {
      if(score_board[i].A) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateA = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateA);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].a);
      }
      if(score_board[i].B) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateB = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateB);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].b);
      }
      if(score_board[i].C) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateC = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateC);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].c);
      }
      if(score_board[i].D) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateD = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateD);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].d);
      }
      if(score_board[i].E) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateE = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateE);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].e);
      }
      if(score_board[i].F) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateF = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateF);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].f);
      }
      if(score_board[i].G) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateG = glm::translate (glm::vec3(score_board[i].x_shift, score_board[i].y_shift, 0));
        Matrices.model *= (translateG);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(score_board[i].g);
      }
    }
  }

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    int i;

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
                for(i=0;i<3;i++) {
                  if(block[i].status) {
                    if(getAxis() != "x")
                      block[i].revolve_block("left");
                    else
                      block[i].revolve_block("up");
                  }
                }
                break;
            case GLFW_KEY_RIGHT:
                for(i=0;i<3;i++) {
                  if(block[i].status) {
                    if(getAxis() != "x")  
                      block[i].revolve_block("right");
                    else
                      block[i].revolve_block("down");
                  }
                }
                break;
            case GLFW_KEY_UP:
                for(i=0;i<3;i++) {
                  if(block[i].status)
                    if(getAxis() != "x")
                      block[i].revolve_block("up");
                    else
                      block[i].revolve_block("right");
                }
                break;
            case GLFW_KEY_DOWN:
                for(i=0;i<3;i++) {
                  if(block[i].status)
                    if(getAxis() != "x")
                      block[i].revolve_block("down");
                    else
                      block[i].revolve_block("left");
                }
                break;
            case GLFW_KEY_T:
                tower_view=1;
                level_view=0;
                top_view=0;
                block_view=0;
                front_view=0;
                break;
            case GLFW_KEY_L:
                level_view=1;
                tower_view=0;
                top_view=0;
                block_view=0;
                front_view=0;
                break;
            case GLFW_KEY_U:
                top_view=1;
                target_mouse_x=0.4*5;
                target_mouse_y=0.4*5;
                tower_view=0;
                level_view=0;
                block_view=0;
                front_view=0;
                break;
            case GLFW_KEY_B:
                block_view=1;
                top_view=0;
                tower_view=0;
                level_view=0;
                front_view=0;
                break;
            case GLFW_KEY_F:
                block_view=0;
                top_view=0;
                tower_view=0;
                level_view=0;
                front_view=1;
                break;
            case GLFW_KEY_C:
                changeAxis();
                break;
            default:
                break;
        }
    }
}

class Level {
  public:
    int stop_switch_x;
    int stop_switch_y;

    void create_level_1() {
      int i, j;

      tiles[8][8].create(0, 1, 1);
      for(i=0;i<10;i++) {
        for(j=0;j<10;j++) {
          if(j!=8||i!=8) {
            tiles[i][j].create(0, 0, 0);
          }
          tiles[i][j].x=tiles[i][j].width*i;
          tiles[i][j].y=tiles[i][j].length*j;
        }
      }


      for(i=0;i<7;i++) {
        for(j=3;j<10;j++) {
          tiles[i][j].status=0;
        }
      }
      for(i=3;i<7;i++) {
        tiles[i][2].status=0;
        tiles[i][0].status=0;
      }
      for(i=3;i<7;i++) {
        tiles[9][i].status=0;
        tiles[7][i].status=0;
      }
    }

    void create_level_2() {
      int i, j;

      for(i=0;i<10;i++) {
        for(j=0;j<10;j++) {
          tiles[i][j].create(0, 0, 0);
        }
      }

      this->stop_switch_x=-1;
      this->stop_switch_y=-1;

      tiles[9][9].create(0, 1, 1);
      tiles[9][0].create(1, 0, 0);
      tiles[4][7].create(1, 0, 0);

      for(i=4;i<9;i++)
        tiles[0][i].create(0, 0, 1);

      for(i=2;i<7;i++)
        tiles[4][i].create(0, 0, 1);

      for(i=0;i<10;i++) {
        for(j=0;j<10;j++) {
          tiles[i][j].status=0;
        }
      }

      for(i=0;i<4;i++) {
        for(j=0;j<4;j++) {
          tiles[i][j].status=1;
        }
      }

      for(i=4;i<9;i++)
        tiles[i][0].status=1;
      for(i=0;i<9;i++)
        tiles[i][9].status=1;
      tiles[9][9].status=1;
      tiles[9][0].status=1;
      tiles[4][7].status=1;

      for(i=0;i<10;i++) {
        for(j=0;j<10;j++) {
          tiles[i][j].x=tiles[i][j].width*i;
          tiles[i][j].y=tiles[i][j].length*j;
        }
      }
    }

    void switch_level_2() {
      int i,j;
      if(this->stop_switch_x!=currIndexX||this->stop_switch_y!=currIndexY) {
        this->stop_switch_x=currX;
        this->stop_switch_y=currY;
        if(isOnTile(9, 0)) {
          if(tiles[9][0].toggle_swtich) {
            for(i=2;i<7;i++) {
              tiles[4][i].status=0;
            }
            tiles[9][0].toggle_swtich=0;
          }
          else {
            for(i=2;i<7;i++) {
              tiles[4][i].status=1;
            } 
            tiles[9][0].toggle_swtich=0;
          }
        }
        else if(isOnTile(4, 7)) {
          if(tiles[4][7].toggle_swtich) {
            for(i=4;i<9;i++) {
              tiles[0][i].status=0;
            }
            tiles[4][7].toggle_swtich=0;
          }
          else {
            for(i=4;i<9;i++) {
              tiles[0][i].status=1;
            }
            tiles[4][7].toggle_swtich=0;
          }
        }
      }
    }
};

Level levels;

void updateClock () {
  total_time++;
  score_board[2].create(3+0.3, 0, total_time%10);
  score_board[3].create(3, 0, total_time/10);
}

void updateGameStatus() {

  updateScore (-3, 0);

  int i, j;
  for(i=0;i<10;i++) {
    for(j=0;j<10;j++) {
      if(tiles[i][j].status) {
        if(tiles[i][j].is_finish) {
          if(abs(currX-tiles[i][j].x)<0.1&&abs(currY-tiles[i][j].y)<0.1) {
            DYING=1;
          }
        }
      }
    }
  }

  switch (currLevel) {
    case 2:
      levels.switch_level_2();
      break;
    default:
      break;
  }
}

void changePos(float X, float Y, string axis) {
  int i;
  for(i=0;i<3;i++) {
    block[i].status=0;
    if(block[i].name == axis) {
      block[i].x=X;
      block[i].y=Y;
      block[i].status=1;
    }
  }
  currX=X;
  currY=Y;
}

void createGame() {
  if(change_level) {
  block[0].create(0.4, 0.4, 0.8, "z");
  block[1].create(0.4, 0.8, 0.4, "y");
  block[2].create(0.8, 0.4, 0.4, "x");
  changePos(0.4*1, 0.4*1, "z");
  target_mouse_x=0.4*5;
  target_mouse_y=0.4*5;
  DYING=0;
  DYING_inc=0;
  DYING_rot=3;
  X_POS=0;
  X_NEG=0;
  Y_POS=0;
  Y_NEG=0;
  mouse_hit=0;
    switch (currLevel) {
      case 1:
        levels.create_level_1();
        break;
      case 2:
        levels.create_level_2();
        break;
      case 3:
        exit(1);
        break;
    }
    change_level=0;
  }
}

void checkGameStatus(GLFWwindow* window) {
  int i, j;

  if(DYING_inc>10)
      change_level=1;

  if(currX>3.9||currX<0||currY>3.9||currY<0) {
    DYING=1;
  }

  for(i=0;i<10;i++) {
    for(j=0;j<10;j++) {
      if(!tiles[i][j].status) {
        if(abs(currX-tiles[i][j].x)<0.3&&abs(currY-tiles[i][j].y)<0.3) {
          DYING=1;
        }
      }
    }
  }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties *
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  int i, j;
  /* Objects should be created before any other gl function and shaders */
  // Create the models
  createTriangle(); // Generate the VAO, VBOs, vertices data & copy into the array buffer

  tower_view=1;
  level_view=0;
  top_view=0;
  block_view=0;
  front_view=0;
  total_score=0;
  total_time=0;

  currAxis[0]=0;
  currAxis[1]=1;
  currAxis[2]=0;

  updateScore (-3, 0);

  total_score=0;

  TIME_X=0;
  TIME_Y=-2;
  TIME_Z=0;

  SCORE_X=0;
  SCORE_Y=-2;
  SCORE_Z=0;

  currX=0.4;
  currY=0.4;

  currIndexX=1;
  currIndexY=1;

  change_level=1;
  currLevel=1;

  updateClock();
  
  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

  
  reshapeWindow (window, width, height);

    // Background color of the scene
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  int width = 800;
  int height = 600;

    GLFWwindow* window = initGLFW(width, height);

    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 2800;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "doc.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

  initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window, 1);
        draw(window, 0);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);

        glfwSetScrollCallback(window, scroll_callback);

        /* decode and play */
        if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
            ao_play(dev, (char*)buffer, done);
        else mpg123_seek(mh, 0, SEEK_SET); // loop audio from start again if ended

        createGame();
        checkGameStatus(window);
        updateGameStatus();
        getCurrIndex();
        // playmusic();
        // printf("%f %f\n", getMouseCoordX(), getMouseCoordY());

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds

        if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            updateClock();
            last_update_time = current_time;
        }
    }

        /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}

   