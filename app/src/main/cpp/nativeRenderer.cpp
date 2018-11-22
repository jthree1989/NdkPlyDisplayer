#include <jni.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <GLES2/gl2.h>

#include "com_mvcn_ndkplyplayer_MyRenderer.h"
#include "mylog.h"
#include "tinyply.h"
#include "matrix.h"
using namespace tinyply;
using namespace std;

// shared_ptr for array
template <typename T>
shared_ptr<T> make_shared_array(size_t size)
{
    //default_delete is the default deleter in STL
    return shared_ptr<T>(new T[size], default_delete<T[]>());
}

#ifdef __cplusplus
extern "C"
{
#endif
/*
 *  Global variables
 * */
GLuint gProgram;
GLuint gmMVPMatrixHandle;
GLuint gvPositionHandle;
GLuint gvNormalHandle;
GLuint gvColorHandle;
GLuint gBufferIdx[3];
vector<float> gVertexBuffer;
vector<unsigned char> gColorBuffer;
vector<float> gNormalBuffer;
const int COORDS_PER_VERTEX = 3;
const int COORDS_PER_COLOR = 4;
int gWidth, gHeight;
int gVertexCount;
// Tinyply treats parsed data as untyped byte buffers. See below for examples.
std::shared_ptr<PlyData> vertices, normals, colors;
/*
 * Helper functions
 * */
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    MANTIS_LOG(LOG_ERROR) << "Could not compile shader "<< "\n"<<  shaderType << buf << "\n";
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    MANTIS_LOG(LOG_ERROR) << "Could not link program: "<< "\n"<< buf << "\n";
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

void read_ply_file(const std::string & filepath)
{
    try
    {
        std::ifstream ss(filepath, std::ios::binary);
        if (ss.fail()) throw std::runtime_error("failed to open " + filepath);

        PlyFile file;
        file.parse_header(ss);

        MANTIS_LOG(LOG_INFO) << "........................................................................\n";
        for (auto c : file.get_comments())
            MANTIS_LOG(LOG_INFO) << "Comment: " << c << "\n";
        for (auto e : file.get_elements())
        {
            MANTIS_LOG(LOG_INFO) << "element - " << e.name << " (" << e.size << ")" << "\n";
            for (auto p : e.properties)
                MANTIS_LOG(LOG_INFO) << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << "\n";
        }
        MANTIS_LOG(LOG_INFO) << "........................................................................\n";


        // The header information can be used to programmatically extract properties on elements
        // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // like vertex position are hard-coded:
        try
        {
            vertices = file.request_properties_from_element("vertex", { "x", "y", "z" });
        }
        catch (const std::exception & e)
        {
            MANTIS_LOG(LOG_ERROR) << "tinyply exception: " << e.what() << "\n";
        }

        try
        {
            normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" });
        }
        catch (const std::exception & e)
        {
            MANTIS_LOG(LOG_ERROR)<< "tinyply exception: " << e.what() << "\n";
        }

        try
        {
            colors = file.request_properties_from_element("vertex", { "red", "green", "blue" });
        }
        catch (const std::exception & e)
        {
            MANTIS_LOG(LOG_ERROR) << "tinyply exception: " << e.what() << "\n";
        }

        file.read(ss);

        if (vertices)
            MANTIS_LOG(LOG_INFO) << "\tRead " << vertices->count << " total vertices "<< "\n";
        if (normals)
            MANTIS_LOG(LOG_INFO) << "\tRead " << normals->count << " total vertex normals " << "\n";
        if (colors)
            MANTIS_LOG(LOG_INFO) << "\tRead " << colors->count << " total vertex colors " << "\n";

        // Type casting to your own native types
        const size_t numVerticesBytes = vertices->buffer.size_bytes();
        gVertexBuffer = vector<float>(vertices->count * 3);
        std::memcpy(gVertexBuffer.data(), vertices->buffer.get(), numVerticesBytes);
        std::vector<float>::iterator maxVertex = std::max_element(std::begin(gVertexBuffer), std::end(gVertexBuffer));
        for(auto& v:gVertexBuffer) v /= *maxVertex;


        const size_t numNormalsBytes = normals->buffer.size_bytes();
        gNormalBuffer = vector<float>(normals->count * 3);
        std::memcpy(gNormalBuffer.data(), normals->buffer.get(), numNormalsBytes);

        const size_t numColorsBytes = colors->buffer.size_bytes();
        gColorBuffer = vector<unsigned char>(colors->count * 3);
        std::memcpy(gColorBuffer.data(), colors->buffer.get(), numColorsBytes);

    }
    catch (const std::exception & e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}
/*
 *  Native JNI functions
 * */
JNIEXPORT jboolean JNICALL
Java_com_mvcn_ndkplyplayer_MyRenderer_nativeInitGLES20(JNIEnv *env, jobject ,
                                                       jstring vertexShaderStr, jstring fragmentShaderStr)
{

    //glDisable(GL_DITHER);					// disable dither to improve performance with reduced quality
    glClearColor(0.0f, 0.0f, 0.0f, 0.4f);	// set clear value for color buffer as black
    //glEnable(GL_CULL_FACE);					// enabled for better performance
    //glClearDepthf(1.0f);					// set clear value [0, 1] for depth buffer as farthest
    //glEnable(GL_DEPTH_TEST);				// Only render pixels in the front
    //glDepthFunc(GL_LEQUAL);				// Render if depth is less

    // Read ply file
    string fileName("/sdcard/faces.ply");
    read_ply_file(fileName);
    gVertexCount = vertices->count;
    MANTIS_LOG(LOG_INFO) << "VertexCount: "<< gVertexCount << ",\tVertices number: " << vertices->count << ",\tNormals number: "  << normals->count << ",\tColors number: " <<colors->count <<"\n";
    glGenBuffers(3, gBufferIdx);
    // Vertex buffer
    const size_t numVerticesBytes = vertices->buffer.size_bytes();
    glBindBuffer(GL_ARRAY_BUFFER, gBufferIdx[0]);
    glBufferData(GL_ARRAY_BUFFER, numVerticesBytes, gVertexBuffer.data(), GL_STATIC_DRAW);
#if 1
    // Color buffer
    const size_t numColorsBytes = colors->buffer.size_bytes();
    glBindBuffer(GL_ARRAY_BUFFER, gBufferIdx[1]);
    glBufferData(GL_ARRAY_BUFFER, numColorsBytes, gColorBuffer.data(), GL_STATIC_DRAW);
    // Normal buffer
    //const size_t numNormalsBytes = normals->buffer.size_bytes();
    //glBindBuffer(GL_ARRAY_BUFFER, gBufferIdx[2]);
    //glBufferData(GL_ARRAY_BUFFER, numNormalsBytes, gNormalBuffer.data(), GL_STATIC_DRAW);
#endif
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load shaders
    const char *vetexStr, *fragmentStr;
    vetexStr = env->GetStringUTFChars(vertexShaderStr, NULL);
    fragmentStr = env->GetStringUTFChars(fragmentShaderStr, NULL);
    gProgram = createProgram(vetexStr, fragmentStr);
    if (!gProgram) {
        MANTIS_LOG(LOG_ERROR) << "Could not create program."<< "\n";
        return JNI_FALSE;
    }
    env->ReleaseStringUTFChars(vertexShaderStr, vetexStr);
    env->ReleaseStringUTFChars(fragmentShaderStr, fragmentStr);

    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_mvcn_ndkplyplayer_MyRenderer_nativeDrawGraphics(JNIEnv *env, jobject,
                                                         jfloat angleX, jfloat angleY, jfloat scale)
{
    glClear(GL_COLOR_BUFFER_BIT);
    // Rotate matrix
    float aRotate[16], aModelView[16], aScale[16], aTranslate[16], aPerspective[16], aMVP[16];
    rotate_matrix(angleX, 0.0, 1.0, 0.0, aRotate);      // Rotate around y-axis
    rotate_matrix(angleY, -1.0, 0.0, 0.0, aModelView);   // Rotate around x-axis
    multiply_matrix(aRotate, aModelView, aModelView);
    // Scale
    scale_matrix(scale, scale, scale, aScale);
    multiply_matrix(aScale, aModelView, aModelView);
    // Translate
    translate_matrix(0.0f, 0.0f, -3.5f, aTranslate);
    multiply_matrix(aTranslate, aModelView, aModelView);
    // Perspective
    perspective_matrix(45.0, (float)gWidth/(float)gHeight, 0.1, 100.0, aPerspective);
    multiply_matrix(aPerspective, aModelView, aMVP);
    // Add program to OpenGL ES environment
    glUseProgram(gProgram);
    // Pass position buffer into the shader
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    glBindBuffer(GL_ARRAY_BUFFER, gBufferIdx[0]);
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0);
#if 1
    // Pass position buffer into the shader
    gvColorHandle = glGetAttribLocation(gProgram, "vColor");
    glBindBuffer(GL_ARRAY_BUFFER, gBufferIdx[1]);
    glEnableVertexAttribArray(gvColorHandle);
    glVertexAttribPointer(gvColorHandle, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
#endif
    // Pass the projection and view transformation to the shader
    gmMVPMatrixHandle = glGetUniformLocation(gProgram, "uMVPMatrix");
    // Pass the projection and view transformation to the shader
    glUniformMatrix4fv(gmMVPMatrixHandle, 1, GL_FALSE, aMVP);
    // Draw points
    glDrawArrays(GL_POINTS, 0, gVertexCount);

    // Disable vertex array
    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(gvColorHandle);
    glFlush();
}

JNIEXPORT void JNICALL
Java_com_mvcn_ndkplyplayer_MyRenderer_nativeSurfaceChanged(JNIEnv *env, jobject,
                                                           jint width, jint height)
{
    glViewport(0, 0, width, height);
    gWidth = width;
    gHeight = height;
}

#ifdef __cplusplus
}
#endif


