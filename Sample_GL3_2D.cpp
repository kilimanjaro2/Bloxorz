#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <ao/ao.h>
#include <mpg123.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#define color_bit = #a0d233
#define BITS 8
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

float rectangle_rotation = 0;
float triangle_rotation = 0;
int right_mouse_clicked = 0, left_mouse_clicked = 0;
int level_pass[10]={0}, level_number = 0, level_start[10], level_end[10], horizontal_angle_moved = 0, vertical_angle_moved = 0, torsion_angle_moved = 0;
int standing_bit = 1, move_left = 0, move_right = 0, move_up = 0, move_down = 0,sleeping_x = 0, sleeping_z = 0, move_clock = 0, move_anti = 0;
int next_left = 90, next_right = -90, next_up = 90, next_down =-90, hor_count = 0, ver_count = 0, next_clock = 90, next_anti = -90,rot_count = 0;
float tile_rotation = 0;
float cameraxdef = 5, cameraydef = 4, camerazdef = 5, camerax = cameraxdef, cameray = cameraydef, cameraz = camerazdef;
float targetx = 0, targety = 0, targetz = 0;
int vis = 0, blockview = 0, defview = 1, topview = 0, blockangle = 90, followview = 0;
float camera_zoom = 0.2;
float camera_rotation_angle = 90;
int in_animation = 0, is_falling = 0,view_number = 1, time_counter = 0,moves_counter = 0;
vector <float> finish_tile_x ={3.0,3.6,1.8};
vector <float> finish_tile_z ={0,0,0};
vector <float> tile_x = {/*level 0 : 14*/0, 0, 0, -0.60, -0.60, -0.60, 0.60, 0.60, 0.60, 1.20,1.80,2.40,0,3,
                         /*level 1 : 29*/0, 0, 0, -0.60, -0.60, -0.60, 0.60, 0.60, 0.60, 1.20,1.80,2.40,0,3,0,0,0,0.6,1.2,1.8,2.4,3.0,2.4,3.0,2.4,3.0,2.4,3.0,0,
                         /*level 2 : 16*/0, 0, 0, -0.60, -0.60, -0.60, 0.60, 0.60, 0.60, 1.20,1.80,2.40,0,3,3.6,4.2};

vector <float> tile_z = {/*level 0 : 14*/0, 0.60, -0.60, 0, -0.60, 0.60, 0, -0.60, 0.60, 0,0,0,0,3.6,
                         /*level 1 : 29*/0, 0.60, -0.60, 0, -0.60, 0.60, 0, -0.60, 0.60, 0,0,0,0,0,-1.2,-1.8,-2.4,-2.4,-2.4,-2.4,-2.4,-2.4,-1.8,-1.8,-1.2,-1.2,-0.6,-0.6,0,
                         /*level 2 : 16*/0, 0.60, -0.60, 0, -0.60, 0.60, 0, -0.60, 0.60, 0,0,0,0,0,0,0};
string title_string;
struct GLMatrices {
    glm::mat4 projectionO, projectionP;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera_zoom += yoffset / 10;
}


GLuint programID;
int proj_type;
glm::vec3 tri_pos, rect_pos;

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
    //    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

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

void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
        fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if(!GLEW_VERSION_3_3)
        fprintf(stderr, "3.3 version not available\n");
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
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_LEFT:
            moves_counter++;
            move_left = 1;
            //rectangle_rotation -= 90;
            break;
        case GLFW_KEY_RIGHT:
            moves_counter++;
            move_right = 1;
            break;
        case GLFW_KEY_UP:
            moves_counter++;
            move_up = 1;
            break;
        case GLFW_KEY_DOWN:
            moves_counter++;
            move_down = 1;
            break;
        case GLFW_KEY_1:
            view_number = 1;
            break;
        case GLFW_KEY_2:
            view_number = 2;
            break;
        case GLFW_KEY_3:
            view_number = 3;
            break;
        case GLFW_KEY_4:
            view_number = 4;
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

        default:
            break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
    case 'o':
        if(blockview == 1)
        {
            blockangle += 5;
            targetx = 1 * cos(blockangle * M_PI / 180) + rect_pos.x;
            targety = 0;
            targetz = 1 * sin(blockangle * M_PI / 180) + rect_pos.z;
        }
        else
        {
            camera_rotation_angle += 5;
        }
        break;
    case 'p':
        if(blockview == 1)
        {
            blockangle -= 5;
            targetx = 1 * cos(blockangle * M_PI / 180) + rect_pos.x;
            targety = 0;
            targetz = 1 * sin(blockangle * M_PI / 180) + rect_pos.z;
        }
        else
        {
            camera_rotation_angle -= 5;
        }
        break;
    case 'f':
        if(camerax == cameraxdef && cameray == cameraydef && cameraz == camerazdef)
        {
            if(standing_bit == 1)
            {
                camerax = rect_pos.x;
                cameray = rect_pos.y + 0.5;
                cameraz = rect_pos.z;
            }
            else
            {
                camerax = rect_pos.x;
                cameray = rect_pos.y  +0.25;
                cameraz = rect_pos.z;
            }
            blockview = 1;
            defview = 0;
            camera_rotation_angle = 0;
            targetx = 0;
            targety = 0;
            targetz = 0;
        }
        else
        {
            camerax = cameraxdef;
            cameray = cameraydef;
            cameraz = camerazdef;
            blockview = 0;
            defview = 1;
            camera_rotation_angle = 90;
            targetx = 0;
            targety = 0;
            targetz = 0;
        }
        break;
    case 'r':
        if(camerax == cameraxdef && cameray == cameraydef && cameraz == camerazdef)
        {
            camerax = 0;
            cameray = 6;
            cameraz = 0;
            topview = 1;
            defview = 0;
            targetx = 1;
            targety = -0.5;
        }
        else
        {
            camerax = cameraxdef;
            cameray = cameraydef;
            cameraz = camerazdef;
            topview = 0;
            defview = 1;
            camera_rotation_angle = 90;
            targetx = 0;
            targety = 0;
            targetz = 0;
        }
        break;
    case 'b':
        if(camerax == cameraxdef && cameray == cameraydef && cameraz == camerazdef)
        {
            camerax = rect_pos.x - 3;
            cameray = 2;
            cameraz = rect_pos.z;
            targetx = rect_pos.x;
            targetz = rect_pos.z;
            targety = 1.7;
            defview = 0;
            followview = 1;
            camera_rotation_angle = 0;
        }
        else
        {
            camerax = cameraxdef;
            cameray = cameraydef;
            cameraz = camerazdef;
            followview = 0;
            defview = 1;
            camera_rotation_angle = 90;
            targetx = 0;
            targety = 0;
            targetz = 0;
        }
        break;
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
            if (action == GLFW_PRESS){
                left_mouse_clicked=1;
                break;
            }
            if (action == GLFW_RELEASE){
                left_mouse_clicked=0;
                break;
            }
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS){
                right_mouse_clicked=1;
                break;
            }
            if (action == GLFW_RELEASE){
                right_mouse_clicked=0;
                break;
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
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projectionP = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projectionO = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *tile, *rectangle, *finish_tile, *bridge_tile, *teleport_tile;

// Creates the triangle object used in this sample code
void createTile ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        -0.3,-0.06,-0.3, // triangle 1 : begin
        -0.3,-0.06, 0.3,
        -0.3, 0.06, 0.3, // triangle 1 : end

        0.3, 0.06,-0.3, // triangle 2 : begin
        -0.3,-0.06,-0.3,
        -0.3, 0.06,-0.3, // triangle 2 : end

        0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,
        0.3,-0.06,-0.3,

        0.3, 0.06,-0.3,
        0.3,-0.06,-0.3,
        -0.3,-0.06,-0.3,

        -0.3,-0.06,-0.3,
        -0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,

        0.3,-0.06, 0.3,
        -0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,

        -0.3, 0.06, 0.3,
        -0.3,-0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3,-0.06,-0.3,
        0.3, 0.06,-0.3,

        0.3,-0.06,-0.3,
        0.3, 0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3, 0.06,-0.3,
        -0.3, 0.06,-0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,
        -0.3, 0.06, 0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06, 0.3,
        0.3,-0.06, 0.3
    };

    static const GLfloat color_buffer_data [] = {


        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        0.1,0.1,1, // color 0
        0.5,0.4,1, // color 1
        0.2,0.2,1, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    tile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createFinishTile ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        -0.3,-0.06,-0.3, // triangle 1 : begin
        -0.3,-0.06, 0.3,
        -0.3, 0.06, 0.3, // triangle 1 : end

        0.3, 0.06,-0.3, // triangle 2 : begin
        -0.3,-0.06,-0.3,
        -0.3, 0.06,-0.3, // triangle 2 : end

        0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,
        0.3,-0.06,-0.3,

        0.3, 0.06,-0.3,
        0.3,-0.06,-0.3,
        -0.3,-0.06,-0.3,

        -0.3,-0.06,-0.3,
        -0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,

        0.3,-0.06, 0.3,
        -0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,

        -0.3, 0.06, 0.3,
        -0.3,-0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3,-0.06,-0.3,
        0.3, 0.06,-0.3,

        0.3,-0.06,-0.3,
        0.3, 0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3, 0.06,-0.3,
        -0.3, 0.06,-0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,
        -0.3, 0.06, 0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06, 0.3,
        0.3,-0.06, 0.3
    };

    static const GLfloat color_buffer_data [] = {


        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    finish_tile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTeleportTile ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        -0.3,-0.06,-0.3, // triangle 1 : begin
        -0.3,-0.06, 0.3,
        -0.3, 0.06, 0.3, // triangle 1 : end

        0.3, 0.06,-0.3, // triangle 2 : begin
        -0.3,-0.06,-0.3,
        -0.3, 0.06,-0.3, // triangle 2 : end

        0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,
        0.3,-0.06,-0.3,

        0.3, 0.06,-0.3,
        0.3,-0.06,-0.3,
        -0.3,-0.06,-0.3,

        -0.3,-0.06,-0.3,
        -0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,

        0.3,-0.06, 0.3,
        -0.3,-0.06, 0.3,
        -0.3,-0.06,-0.3,

        -0.3, 0.06, 0.3,
        -0.3,-0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3,-0.06,-0.3,
        0.3, 0.06,-0.3,

        0.3,-0.06,-0.3,
        0.3, 0.06, 0.3,
        0.3,-0.06, 0.3,

        0.3, 0.06, 0.3,
        0.3, 0.06,-0.3,
        -0.3, 0.06,-0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06,-0.3,
        -0.3, 0.06, 0.3,

        0.3, 0.06, 0.3,
        -0.3, 0.06, 0.3,
        0.3,-0.06, 0.3
    };

    static const GLfloat color_buffer_data [] = {

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

        1,0.1,0.1, // color 0
        1,0.4,0.5, // color 1
        1,0.2,0.2, // color 2

        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2

        1,0,0, // color 0
        1,0,0, // color 1
        1,0,0, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    teleport_tile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.3,-0.6,-0.3, // triangle 1 : begin
        -0.3,-0.6, 0.3,
        -0.3, 0.6, 0.3, // triangle 1 : end

        0.3, 0.6,-0.3, // triangle 2 : begin
        -0.3,-0.6,-0.3,
        -0.3, 0.6,-0.3, // triangle 2 : end

        0.3,-0.6, 0.3,
        -0.3,-0.6,-0.3,
        0.3,-0.6,-0.3,

        0.3, 0.6,-0.3,
        0.3,-0.6,-0.3,
        -0.3,-0.6,-0.3,

        -0.3,-0.6,-0.3,
        -0.3, 0.6, 0.3,
        -0.3, 0.6,-0.3,

        0.3,-0.6, 0.3,
        -0.3,-0.6, 0.3,
        -0.3,-0.6,-0.3,

        -0.3, 0.6, 0.3,
        -0.3,-0.6, 0.3,
        0.3,-0.6, 0.3,

        0.3, 0.6, 0.3,
        0.3,-0.6,-0.3,
        0.3, 0.6,-0.3,

        0.3,-0.6,-0.3,
        0.3, 0.6, 0.3,
        0.3,-0.6, 0.3,

        0.3, 0.6, 0.3,
        0.3, 0.6,-0.3,
        -0.3, 0.6,-0.3,

        0.3, 0.6, 0.3,
        -0.3, 0.6,-0.3,
        -0.3, 0.6, 0.3,

        0.3, 0.6, 0.3,
        -0.3, 0.6, 0.3,
        0.3,-0.6, 0.3
    };

    static const GLfloat color_buffer_data [] = {

        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
        0.2,0.1,0, // color 0
        0.2,0.4,0.3, // color 1
        0.2,0.5,0.3, // color 2

        0.2,0.4,0.3, // color 1
        0.2,0.1,0, // color 0
        0.2,0.5,0.3, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}



/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, float x, float y, float w, float h)
{
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));

    double new_mouse_x, new_mouse_y;
    glfwGetCursorPos(window, &new_mouse_x, &new_mouse_y);
    if (left_mouse_clicked == 1)
    {
        camera_rotation_angle = (new_mouse_x * 360 / 600.0);
        camerax = cameraxdef;
        cameraz = cameraydef;
    }

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    //    glm::vec3 eye ( /*5*cos(camera_rotation_angle*M_PI/180.0f)*/0, 3, 5/**sin(camera_rotation_angle*M_PI/180.0f)*/ );
    //    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    //    glm::vec3 target (0, 0, 0);
    //    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    //    glm::vec3 up (0, 1, 0);

    glm::vec3 eye(camerax * cos(camera_rotation_angle * M_PI / 180.0f), cameray, cameraz * sin(camera_rotation_angle * M_PI / 180.0f));
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target(targetx, targety, targetz);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up(0, 1, 0);


    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

    if(blockview == 1)
    {
        if(standing_bit == 1)
        {
            camerax = rect_pos.x;
            cameray = rect_pos.y + 0.5;
            cameraz = rect_pos.z;
        }
        else
        {
            camerax = rect_pos.x;
            cameray = rect_pos.y + 0.25;
            cameraz = rect_pos.z;
        }
        targetx = 1 * cos(blockangle * M_PI / 180) + rect_pos.x;
        targety = 0;
        targetz = 1 * sin(blockangle * M_PI / 180) + rect_pos.z;
    }
    else if(defview == 1)
    {
        camerax = cameraxdef;
        cameray = cameraydef;
        cameraz = camerazdef;
        targetx = 0;
        targety = 0;
        targetz = 0;
    }
    else if(topview == 1)
    {
        camerax = 0;
        cameray = 6;
        cameraz = 0;
        targetx = 1;
        targety = -0.5;
    }
    else if(followview == 1)
    {
        camerax = rect_pos.x - 3;
        cameray = 2;
        cameraz = rect_pos.z;
        targetx = rect_pos.x;
        targetz = rect_pos.z;
        targety = 1.7;
        camera_rotation_angle = 0;
    }

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = (proj_type?Matrices.projectionP:Matrices.projectionO) * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    int i;
    //cout << level_number << endl << endl << endl;
    for(i=level_start[level_number];i<level_end[level_number];i++)
    {

        //cout << i << " " << level_end[level_number] << endl;
        // Load identity to model matrix
        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 translateTile = glm::translate (glm::vec3(tile_x[i],0,tile_z[i])); // glTranslatef
        glm::mat4 rotateTile = glm::rotate((float)(tile_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 tileTransform = translateTile * rotateTile;
        Matrices.model *= tileTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(tile);

    }

    title_string = "Level Number: ";

    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */
    glm::mat4 translateFinishTile = glm::translate (glm::vec3(finish_tile_x[level_number],0,finish_tile_z[level_number])); // glTranslatef
    glm::mat4 rotateFinishTile = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 finishTileTransform = translateFinishTile * rotateFinishTile;
    Matrices.model *= finishTileTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(finish_tile);

    if(level_number == 2)
    {
        //cout << "NOOOO" << endl;
        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 translateTeleportTile = glm::translate (glm::vec3(-0.6,0,0.6)); // glTranslatef
        glm::mat4 rotateTeleportTile = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 teleportTileTransform = translateTeleportTile * rotateTeleportTile;
        Matrices.model *= teleportTileTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(teleport_tile);
    }

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();
    Matrices.model = glm::mat4(1.0f);
    if(move_left == 1 && horizontal_angle_moved < next_left && standing_bit == 1)
    {
        in_animation = 1;
        if(vis == 0)
        {
            hor_count --;
            vis = 1;
        }
        rect_pos.y -= 0.03;
        rect_pos.x -=0.09;
        horizontal_angle_moved += 9;
        if (horizontal_angle_moved == next_left)
        {
            in_animation = 0;
            move_left = 0;
            standing_bit = 0;
            sleeping_x = 1;
            sleeping_z = 0;
            next_left += 90;
            next_right += 90;
            vis = 0;
        }
    }
    else if(move_left == 1 && horizontal_angle_moved < next_left && /*standing_bit == 0*/ sleeping_x == 1)
    {
        in_animation = 1;
        if(vis == 0)
        {
            hor_count --;
            vis = 1;
        }
        rect_pos.y += 0.03;
        rect_pos.x -=0.09;
        horizontal_angle_moved += 9;
        if (horizontal_angle_moved == next_left)
        {
            in_animation = 0;
            move_left = 0;
            standing_bit = 1;
            next_left += 90;
            next_right +=90;
            sleeping_x = 0;
            sleeping_z = 0;
            vis = 0;
        }
    }
    else if(move_left == 1 && torsion_angle_moved > next_anti && sleeping_z ==1)
    {
        in_animation = 1;
        if(vis==0)
        {
            vis = 1;
            rot_count--;
        }
        rect_pos.x -=0.06;
        torsion_angle_moved -= 9;
        /*if(abs(rot_count)%4==2)
        {
            torsion_angle_moved +=18;
            if(torsion_angle_moved == next_clock)
            {
                move_left = 0;
                next_clock += 90;
                next_anti +=90;
                standing_bit = 0;
                sleeping_x = 0;
                sleeping_z = 1;
            }
        }*/
        if(torsion_angle_moved == next_anti)
        {
            in_animation = 0;
            move_left = 0;
            next_clock -=90;
            next_anti -=90;
            sleeping_x = 0;
            standing_bit = 0;
            vis = 0;
        }
    }
    else if(move_right == 1 && horizontal_angle_moved > next_right && standing_bit == 1)
    {
        in_animation = 1;
        if(vis==0)
        {
            hor_count++;
            vis =1;
        }
        rect_pos.y -= 0.03;
        rect_pos.x += 0.09;
        horizontal_angle_moved -= 9;
        if(horizontal_angle_moved == next_right)
        {
            in_animation = 0;
            standing_bit = 0;
            move_right = 0;
            next_right -= 90;
            next_left -= 90;
            sleeping_x = 1;
            sleeping_z = 0;
            vis = 0;
        }
    }
    else if(move_right == 1 && horizontal_angle_moved > next_right && sleeping_x == 1)
    {
        in_animation = 1;
        if(vis == 0)
        {
            hor_count ++;
            vis = 1;
        }
        rect_pos.y += 0.03;
        rect_pos.x += 0.09;
        horizontal_angle_moved -= 9;
        if(horizontal_angle_moved == next_right)
        {
            in_animation = 0;
            standing_bit = 1;
            move_right = 0;
            next_right -= 90;
            next_left -= 90;
            sleeping_x = 0;
            sleeping_z = 0;
            vis = 0;
        }
    }
    else if(move_right == 1 && torsion_angle_moved < next_clock && sleeping_z == 1)
    {
        in_animation = 1;
        if(vis==0)
        {
            rot_count++;
            vis = 1;
        }
        rect_pos.x +=0.06;
        torsion_angle_moved += 9;
        if(torsion_angle_moved == next_clock)
        {
            in_animation = 0;
            move_right = 0;
            next_clock += 90;
            next_anti += 90;
            sleeping_x = 0;
            standing_bit = 0;
            vis = 0;
        }
    }
    else if(move_up == 1 && vertical_angle_moved > next_down && standing_bit == 1)
    {
        in_animation = 1;
        rect_pos.z -=0.09;
        rect_pos.y -=0.03;

        vertical_angle_moved -= 9;
        if(abs(hor_count)%4==2)
        {
            vertical_angle_moved +=18;
            if(vertical_angle_moved == next_up)
            {
                in_animation = 0;
                move_up = 0;
                next_up += 90;
                next_down +=90;
                standing_bit = 0;
                sleeping_x = 0;
                sleeping_z = 1;
                vis = 0;
            }
        }
        if(vertical_angle_moved == next_down)
        {
            in_animation = 0;
            standing_bit = 0;
            sleeping_x = 0;
            sleeping_z = 1;
            move_up = 0;
            next_up -=90;
            next_down -=90;
        }
    }
    else if(move_up == 1 && vertical_angle_moved > next_down && sleeping_z == 1)
    {
        in_animation = 1;
        rect_pos.z -= 0.09;
        rect_pos.y += 0.03;
        //cout << "LOL" << endl;
        vertical_angle_moved -=9;
        if(abs(hor_count)%4==2)
        {
            vertical_angle_moved +=18;
            if(vertical_angle_moved == next_up)
            {
                in_animation = 0;
                move_up = 0;
                next_up += 90;
                next_down +=90;
                standing_bit = 1;
                sleeping_x = 0;
                sleeping_z = 0;
                vis = 0;
            }
        }
        if(vertical_angle_moved == next_down)
        {
            in_animation = 0;
            standing_bit = 1;
            sleeping_x = 0;
            sleeping_z = 0;
            move_up = 0;
            next_up -=90;
            next_down -=90;
        }
    }
    else if(move_up == 1 && torsion_angle_moved < next_clock && sleeping_x == 1)
    {
        in_animation = 1;
        rect_pos.z -=0.06;
        torsion_angle_moved += 9;
        if(torsion_angle_moved == next_clock)
        {
            in_animation = 0;
            move_up = 0;
            next_clock+=90;
            next_anti +=90;
        }
    }
    else if(move_down ==1 && vertical_angle_moved < next_up && standing_bit == 1)
    {
        in_animation = 1;
        rect_pos.z += 0.09;
        rect_pos.y -= 0.03;
        vertical_angle_moved +=9;
        if(abs(hor_count)%4==2)
        {
            vertical_angle_moved -=18;
            if(vertical_angle_moved == next_down)
            {
                in_animation = 0;
                standing_bit = 0;
                sleeping_z = 1;
                sleeping_x = 0;
                move_down = 0;
                next_up-=90;
                next_down -=90;
                vis = 0;
            }
        }
        if(vertical_angle_moved == next_up)
        {
            in_animation = 0;
            standing_bit = 0;
            sleeping_x = 0;
            sleeping_z = 1;
            move_down = 0;
            next_up +=90;
            next_down +=90;
        }
    }
    else if(move_down == 1 && vertical_angle_moved < next_up && sleeping_z == 1)
    {
        in_animation = 1;
        rect_pos.z +=0.09;
        rect_pos.y +=0.03;
        vertical_angle_moved +=9;
        if(abs(hor_count)%4==2)
        {
            vertical_angle_moved -=18;
            if(vertical_angle_moved == next_down)
            {
                in_animation = 0;
                standing_bit = 1;
                sleeping_x = 0;
                sleeping_z = 0;
                move_down = 0;
                next_up-=90;
                next_down -=90;
                vis = 0;
            }
        }
        if(vertical_angle_moved == next_up)
        {
            in_animation = 0;
            standing_bit =1;
            sleeping_x = 0;
            sleeping_z = 1;
            move_down = 0;
            next_up +=90;
            next_down +=90;
        }
    }
    else if(move_down == 1 && torsion_angle_moved > next_anti && sleeping_x == 1)
    {
        in_animation = 1;
        //cout << next_anti << endl << vertical_angle_moved << endl << next_clock << endl << "lol" << endl;
        rect_pos.z +=0.06;
        torsion_angle_moved -= 9;
        if(torsion_angle_moved == next_anti)
        {
            in_animation = 0;
            move_down = 0;
            next_clock-=90;
            next_anti -=90;
        }
    }
    //cout << hor_count << " " << ver_count << endl;
    //cout << rect_pos.x <<endl << finish_tile_x[level_number] << endl << endl<< rect_pos.z << endl <<finish_tile_z[level_number]<< endl;

    // cout << "angle_moved " << vertical_angle_moved  << endl << "next_up " << next_up << endl <<"next_down " << next_down << endl;
    // cout << "x " << rect_pos.x << endl << "y" << rect_pos.y << endl <<"z" << rect_pos.z << endl;
    rectangle_rotation = horizontal_angle_moved;
    glm::mat4 translateRectangle = glm::translate (rect_pos);        // glTranslatef
    glm::mat4 rotateRectangle = glm::rotate((float)(horizontal_angle_moved*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    glm::mat4 rotateRectanglex = glm::rotate((float)(vertical_angle_moved*M_PI/180.0f), glm::vec3(1,0,0));
    glm::mat4 rotateRectangley = glm::rotate((float)(torsion_angle_moved*M_PI/180.0f), glm::vec3(0,1,0));
    Matrices.model *= (translateRectangle * rotateRectangle * rotateRectanglex *rotateRectangley);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle);

    // Increment angles
    float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    //  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    //  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        exit(EXIT_FAILURE);
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createTile ();// Generate the VAO, VBOs, vertices data & copy into the array buffer
    createFinishTile();
    createRectangle ();
    createTeleportTile();

    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    // cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    // cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    // cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    // cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void reset()
{
    rect_pos.x = -0.6;
    rect_pos.y = 0.66;
    rect_pos.z = 0;
    hor_count = 0;
    ver_count = 0;
    rot_count = 0;
    standing_bit = 1, move_left = 0, move_right = 0, move_up = 0, move_down = 0,sleeping_x = 0, sleeping_z = 0, move_clock = 0, move_anti = 0;
    horizontal_angle_moved = 0, vertical_angle_moved = 0, torsion_angle_moved = 0;
    next_left = 90, next_right = -90, next_up = 90, next_down =-90, hor_count = 0, ver_count = 0, next_clock = 90, next_anti = -90,rot_count = 0;
}

void check()
{
    int i;

    if(level_number == 2 && standing_bit == 1)
    {
        if (abs(rect_pos.x + 0.6) <=0.01 && abs(rect_pos.z - 0.6)<=0.01)
        {
            rect_pos.x= 3.6;
            rect_pos.z = 0 ;
        }
    }
    int standing_falling = 0, sleepingx_l=0, sleepingx_r = 0,sleepingz_l = 0,sleepingz_r = 0;
    if(in_animation == 0)
    {
        if(standing_bit == 1)
        {
            for(i=level_start[level_number]; i<level_end[level_number]; i++)
            {
                if(abs(rect_pos.x-tile_x[i])<=0.01 && abs(rect_pos.z-tile_z[i])<=0.01)
                {
                    standing_falling = 1;
                }
            }
            if(standing_falling == 0)
            {
                reset();
            }
        }
        else if(sleeping_x == 1)
        {
            for(i=level_start[level_number]; i<level_end[level_number]; i++)
            {
                if(abs(rect_pos.x-tile_x[i]-0.3)<=0.01 && abs(rect_pos.z-tile_z[i])<=0.01)
                {
                    sleepingx_l = 1;
                    break;
                }
            }
            for(i=level_start[level_number]; i<level_end[level_number]; i++)
            {
                if(abs(rect_pos.x-tile_x[i]+0.3)<=0.01 && abs(rect_pos.z-tile_z[i])<=0.01)
                {
                    sleepingx_r = 1;
                    break;
                }
            }
            if(sleepingx_l == 0 || sleepingx_r == 0)
            {
                reset();
            }
        }
        else if(sleeping_z == 1)
        {
            for(i=level_start[level_number]; i<level_end[level_number]; i++)
            {
                if(abs(rect_pos.x-tile_x[i])<=0.01 && abs(rect_pos.z-tile_z[i]-0.3)<=0.01)
                {
                    sleepingz_l = 1;
                    break;
                }
            }
            for(i=level_start[level_number]; i<level_end[level_number]; i++)
            {
                if(abs(rect_pos.x-tile_x[i])<=0.01 && abs(rect_pos.z-tile_z[i]+0.3)<=0.01)
                {
                    sleepingz_r = 1;
                    break;
                }
            }
            if(sleepingz_l == 0 || sleepingz_r == 0)
            {
                reset();
            }
        }
    }

}



int main (int argc, char** argv)
{

    //music_initialise();
    //level_number= 2;
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

    // if(argc < 2)
    //   exit(0);

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 3200;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "q.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    int width = 600;
    int height = 600;
    proj_type = 1;
    tri_pos = glm::vec3(0, 0, 0);
    rect_pos = glm::vec3(-0.6, 0.66, 0);
    level_start[0] = 0;
    level_end[0] = 13;
    level_start[1] = 14;
    level_end[1] =43;
    level_start[2] = 43;
    level_end[2] = 58;
    level_start[3] = 58;
    level_end[3] = 85;



    GLFWwindow* window = initGLFW(width, height);
    initGLEW();
    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        string str1 = to_string(level_number + 1);
        string str2 = to_string(time_counter);
        string str3 = to_string(moves_counter);
        title_string = "Level Number: " + str1 + " TIME: " + str2 + " Moves: " + str3;
        const char *feeder = title_string.c_str();
        glfwSetWindowTitle(window, feeder);
        // clear the color and depth in the frame buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
            ao_play(dev, (char*) buffer, done);
        else mpg123_seek(mh, 0, SEEK_SET);

        // OpenGL Draw commands
        draw(window, 0, 0, 1, 1);
        // proj_type ^= 1;
        // draw(window, 0.5, 0, 0.5, 1);
        // proj_type ^= 1;
        //cout << fabs(rect_pos.z - finish_tile_z[level_number]);
        if(standing_bit==1 && fabs(rect_pos.x - finish_tile_x[level_number])<=0.001 && fabs(rect_pos.z - finish_tile_z[level_number])<=0.001)
        {
            level_number++;
            reset();
        }
        int i;

        check();


        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            time_counter +=1;
            last_update_time = current_time;
        }
    }
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
    glfwTerminate();
    return 0;
    //    exit(EXIT_SUCCESS);
}
