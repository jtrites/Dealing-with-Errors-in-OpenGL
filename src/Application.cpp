#include <GL/glew.h>  /* must be the 1st #include BEFORE ANY other #includes */
#include <GLFW/glfw3.h>
#include <iostream>

// Vid#8 add includes to read, parse C++ external file: Basic.shader, and add to each Shader buffer
#include <fstream> 
#include <string>
#include <sstream>

/* Vid#10: (14:30) add ASSERT(x) macro to validate a condition and call a breakpoint if true 
    using the MSVC function __debugbreak */

#define ASSERT(x) if (!(x)) __debugbreak();

/* Vid#10: (16:20) GLCall(x) macro where (x) is the call function to Clear OpenGL Error(s)
    that calls the GLClearErrors() function */
/* Vid#10 (18:45) use macros to find out which line of code this errored function occurred.
    In GLLogCall(x) - changed to a string (#x) for printing the file name (__FILE__),
    and printing the line number (__LINE__) */

#define GLCall(x) GLClearErrors();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

/* Vid10: add new GLClearError() static function that returns void */
static void GLClearErrors()
{
    /* loop while there are errors and until GL_NO_ERROR is returned */
    while (glGetError() != GL_NO_ERROR);
}

/* Vid10: add new GLCheckErrors() static function that returns unsigned enum (int) in order */
/* (14:00) change GLCheckErrors() --> GLLogCall() */
/* (17:45) modify GLLogCall to accept parameters that allow the console 
    to printout the C++ source file, the line of code, and  the function name that errored */
static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") " << function 
            << " " << file << ": " << line << std::endl;
        return false;
    }

    return true;
}

/*** Create a struct that allows returning multiple items ***/
struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

/*** Vid#8 Add new function ParseShader to parse external Basic.shader file 
    returns - struct ShaderProgramSource above which contains two strings (variables)
    note: C++ functions are normally capable of only returning one variable ***/
static ShaderProgramSource ParseShader(const std::string& filepath)
{
    /* open file */
    std::ifstream stream(filepath);

    /* create enum class for each Shader type */
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    /* parse file line by line */
    std::string line;

    /* define buffers for 2 Shaders:  vertexShader and fragmentShader */
    std::stringstream ss[2];    

    /* set initial ShaderType = NONE */
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        /* find "#shader" keyword */
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                /* set mode to vertex */
                type = ShaderType::VERTEX;

            else if (line.find("fragment") != std::string::npos)
                /* set mode to fragment */
                type = ShaderType::FRAGMENT; 
        }
        else
            /* add each line to the corresponding buffer after detecting the ShaderType */
        {
            /* type is an index to push data into the selected array buffer, casted to a Shader int type,
                to add each new line plus newline char */
            ss[(int)type] << line << '\n';
        }
    }

    /* returns a struct comprised of two ss strings */
    return { ss[0].str(), ss[1].str() };
}

/*** Vid#7: create static int CreateShader function with parameters:
    unsigned int type (used raw C++ type instead of OpenGL GLuint type to allow other non-OpenGL GPU driver implementations), 
    const std::string& source
    returns a static unsigned int, takes in a type and a string ptr reference to a source ***/
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    /* change GL_VERTEX_SHADER to type */
    unsigned int id = glCreateShader(type);

    /* returns a char ptr* src to a raw string (the beginning of our data) 
        assigned to source which needs to exist before this code is executed 
        pointer to  beginning of our data */
    const char* src = source.c_str();   

    /* specify glShaderSource(Shader ID, source code count, ptr* to memory address of ptr*, length) 
        as the source of our Shader */
    glShaderSource(id, 1, &src, nullptr);

    /* specify glCompileShader(Shader ID), then return the Shader ID */
    glCompileShader(id);

    /*error handling - query void glGetShaderiv(GLuint shader, GLenum pname, GLint *params); 
        i - specifies an integer
        v - specifies a vector (array) */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        /* query message - length and contents 
           void glGetShaderiv(GLuint shader, GLenum pname, GLint *params); */
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        /* construct char message[length] array allocated on the stack */
        char* message = (char*)alloca(length * sizeof(char));

        /* glGetShaderInfoLog — Returns the information log for a shader object 
            void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); */
        glGetShaderInfoLog(id, length, &length, message);

        /* print the message to the console using std::cout */
        std::cout << "Failed to Compile " 
            << (type == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader")
            << std::endl; 
        std::cout << message << std::endl;

        /* delete Shader using id and return error code = 0 */
        glDeleteShader(id);
        return 0;
    }

    return id;
}

/*** Vid#7: create static int CreateShader function with parameters:
    const string pointer vertexShader(actual source code),
    const string pointer fragmentShader (actual source code)
    returns a static int, takes in the actual source code of these two Shader strings ***/
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    /* glCreateProgram() return an unsigned int program */
    unsigned int program = glCreateProgram();

    /* create vertexShader object */
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);

    /* create fragmentShader object */
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    /* attach vs and fs Shader files, link and validate them to our program ID 
       void glAttachShader(GLuint program, GLuint shader); */
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    /* void glLinkProgram(GLuint program); */
    glLinkProgram(program);

    /* void glValidateProgram(	GLuint program); */
    glValidateProgram(program);

    /* finally, delete the intermediary *.obj files (objects vs and fs) of program ID
        and return an unsigned int program 
        void glDeleteShader(GLuint shader); */
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /*** Make the window's context current - this MUST BE PEFORMED BEFORE glewInit() !!! ***/
    glfwMakeContextCurrent(window);

    /*** Vid#3: JT Added Modern OpenGL code here - MUST FOLLOW glfwMakeContextCurrent(window) ***/
    if (glewInit() != GLEW_OK)
        std::cout << "glewInit() Error!" << std::endl;

    /*** Vid#3: JT Added Print Modern OpenGL Version code here ***/
    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Vid#9A: add 2nd set of (3) x, y, z vertex positions for 2nd inverted triangle added 
            to original right triangle forming a new Rectangle */
    /* Vid#8: modified to (3) x, y, and z vertex positions per LearnOpenGL */
    /* Vid#4: JT Define Vertex Buffer code based on Vid#2 example commmented out below */
    /* create float array of [3] verticies - (3) x, y, z vertex position pairs by Alt+Shift Legacy vertices --> Ctrl+c */
 /*   float positions[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };
*/
    /* Vid#9B: remove 2 duplicate vertices of the 6 vertices in position[] to implement
            an Index Buffer */
    float positions[] = {
        -0.5f, -0.5f, 0.0f, // vertex 0
         0.5f, -0.5f, 0.0f, // vertex 1
         0.5f,  0.5f, 0.0f, // vertex 2
        -0.5f,  0.5f, 0.0f, // vertex 3
    };
    
    /* Vid9B: create Index Buffer using new indices[] array 
        note: must be unsigned but can use char, short, int, etc. */
    unsigned int indices[] = {
        0, 1, 2,        // 1st right triangle drawn CCW
        2, 3, 0         // 2nd inverted right triangle drawn CCW
    };

/* Vid5: original glGenBuffers(), glBindBuffer(), and glBufferData() calls */
    /* glGenBuffer(int bufferID, pointer to memory address of unsigned int buffer) creates buffer and provides and ID */
/* Vid10: (20:30) wrap GLCall() around these (3) gl calls */
    
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));

    /* Bind or Select Buffer which is the target (type = GL_ARRAY_BUFFER, ID = buffer)  */
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));

    /* Specify the type, size of data to be placed into the buffer */
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW));

/* Vid9: new Index Buffer calls */
    /* glGenBuffer(int bufferID, pointer to memory address of unsigned int buffer) creates buffer and provides and ID */
/* Vid10: (20:30) wrap GLCall() around these (3) gl calls */

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));

    /* Bind or Select Buffer which is the target (type = GL_ELEMENT_ARRAY_BUFFER, ID = ibo)  */
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

    /* Specify the type, size of data to be placed into the buffer */
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));


    /*** Vid#5 - OpenGL Vertex Attributes - use attribute pointers to GPU Memory Layout
        for each primitive type that to be drawn on the screen ***/
        /* Enable or disable a generic vertex attribute array for index = 0 */
    glEnableVertexAttribArray(0);

    /* define an array of generic vertex attribute data
        index = 0 1st param,
        size = 3 2nd param for a (3) component vector that represents each Vertex position,
        symbolic constant = GL_FLOAT 3rd param,
        normalized = converted directly as fixed-point values (GL_FALSE) 4th param,
        stride = the amount of bytes between each Vertex based on 
            2nd param vec2 (x, y, z) component position vector of 3 floats = 12 bytes,
        pointer = position has an offset pointer = 0 */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    
    /* Vid#8: (15:30) - replace with new Basic.shader test code */
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;

    /*** Vid#8 (15:10) - temporarily commented out for 1st test new 
        Basic.shader file and supporting Application.cpp code ***/

    /* Call to create vertexShader and fragmentShader above */
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    /* Bind our Shader */
    glUseProgram(shader);


    /* Games Render Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Vid#9: modify buffer = 3 to buffer = 6 for (2) adjacent Triangles forming a Rectangle */
        /* Vid#4: Modern OpenGL draws what's in the new Vertex Buffer */
        /* Two Draw Pull Methods:
            Method 1:  void glDrawArrays(primitive = GL_TRIANGLES, starting vertices = 0, vertice count = 3);
            Method 2:  void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
            glDrawArrays(GL_TRIANGLES, 0, 3); draws a Triangle based on the last glBindBuffer(GL_ARRAY_BUFFER, buffer); */

        /* Vid#4 original call to glDrawArrays(GL_TRIANGLES, 0, #indices)*/
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        /*** Vid#9 new Draw call to glDrawElements(GL_TRIANGLES, #indices, type, ptr to index buffer 
            or nullptr b/c we bound it using glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); ) ***/
        
        /*** Vid#10 Test glGetError() to print error(s) to the console 
            1st - clear previous error(s),
            2nd - call glDrawElements(),
            3rd - call glGetErrors) in while loop and print them sequentially to the console ***/
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Vid#10 (17:30) replace this code with the 2nd ASSERT GLCall wrapped around the glDrawElements() call */
        //GLClearErrors();
        //glDrawElements(GL_TRIANGLES, 6, GL_INT, nullptr);
        //ASSERT(GLLogCall());

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    /* delete Shader */
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}