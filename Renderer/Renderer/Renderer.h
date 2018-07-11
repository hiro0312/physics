#pragma once

#include <Common/Base.h>
#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <math.h>

class Shader;
class Camera;

// Datatypes passed to draw funcs, and used within renderer
struct Vertex2
{
    Vertex2( const float x = 0.f, const float y = 0.f ); // Defaults ( 0.f, 0.f )
    float x, y;
};

struct Vertex3
{
    enum { NUM_FLOATS = 3 };
    Vertex3( const float x = 0.f, const float y = 0.f, const float z = 0.f ); // Defaults ( 0.f, 0.f, 0.f )

    const Vertex3 operator-( const Vertex3& other ) const
    {
        Vertex3 res( x - other.x, y - other.y, z - other.z );
        return res;
    }

    const Vertex3 cross( const Vertex3& other ) const
    {
        Vertex3 res( y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x );
        return res;
    }

    float x, y, z;
};

struct Color
{
    enum { NUM_FLOATS = 4 };
    Color( const float r = 0.f, const float g = 0.f, const float b = 0.f, const float a = 1.f ); // Opaque black default
    float r, g, b, a;
};

struct RendererCinfo
{
    RendererCinfo()
    {
        m_cameraPos = glm::vec3( 0.f, 0.f, -3.f );
        m_cameraDir = glm::vec3( 0.f, 0.f, 1.f );
        m_cameraUp = glm::vec3( 0.f, 1.f, 0.f );

        m_lightColor = glm::vec3( 1.f, 1.f, 1.f );
        m_lightPos = glm::vec3( 0.f, 1.f, 0.f );
        m_backColor = glm::vec3( 0.f, 0.f, 0.f );

        m_fov = 45.f;
        m_nearPlane = .01f;
        m_farPlane = 100.f;
    }

    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraDir;
    glm::vec3 m_cameraUp;
    glm::vec3 m_lightColor;
    glm::vec3 m_lightPos;
    glm::vec3 m_backColor;

    // Defaults 45.f
    float m_fov;

    // Defaults 0.01f
    float m_nearPlane;

    // Defaults 100.f
    float m_farPlane;
};

class Renderer
{
public:

    Renderer();
    ~Renderer();

    // Call this before your loop
    int init( struct GLFWwindow* window, const RendererCinfo& cinfo );

    // Call this in your loop, before the draw funcs
    void prestep();

    // Call this in your loop, after the draw funcs
    void render();

    // Call this after your loop
    int terminate();

    // Camera access, for changing view
    Camera* getCamera() { return m_camera; }

    void addDisplayLine( const Vertex3 a, const Vertex3 b, const Color color = Color() );
    int addDisplayCuboid( const Vertex3 min, const Vertex3 max, const glm::mat4& model = glm::mat4( 1.f ), const Color color = Color() );
    void removeDisplayCuboid( int index );

    // Text
    void drawText2d( const Vertex2 pos, const Color color, const char* string, ... );
    void drawText3d( const Vertex3 pos, const Color color, const char* string, ... );

    void setModel( int index, const glm::mat4& model );

private:

    struct LightSource
    {
        glm::vec3 m_color;
        glm::vec3 m_pos;
    };

    struct DisplayLines
    {
        // One buffer shared for all lines,
        // all lines in world space,
        // single draw call

        enum { MAX_NUM_LINES = 1024 };

        void create();
        void writeBufferLine( const Vertex3 a, const Vertex3 b, const Color color );
        void clearBufferLine( int index );
        void render( const glm::mat4& projection, const glm::mat4& view );

        Shader* m_shader;

        GLuint m_vao;
        GLuint m_vbo[2];

        Vertex3 m_vertices[MAX_NUM_LINES * 2];
        Color m_color[MAX_NUM_LINES * 2];
        int m_numVerts;
    };

    struct DisplayTris
    {
        enum { NUM_MAX_TRIS = 32 };

        struct Triangle
        {
            enum
            {
                NUM_VERTS = 3,
                NUM_FLOATS_FOR_VERTICES = NUM_VERTS * Vertex3::NUM_FLOATS,
                NUM_FLOATS_FOR_COLORS = NUM_VERTS * Color::NUM_FLOATS
            };
        };

        struct Buffer
        {
            float m_vertices[NUM_MAX_TRIS * Triangle::NUM_FLOATS_FOR_VERTICES];
            float m_colors[NUM_MAX_TRIS * Triangle::NUM_FLOATS_FOR_COLORS];
        };
    };

    // Specialized buffer for cuboids
    struct DisplayCuboids
    {
        // One vertex buffer for vert, normal, color for all cuboids
        // One draw call per cuboid for model matrix update
        // TODO: replace color floats /w uniforms

        enum { NUM_INITIAL_MAX_CUBOIDS = 256 };

        // Per-cuboid data
        struct Cuboid
        {
            enum
            {
                NUM_VERTS = 36,
                NUM_FLOATS_FOR_VERTICES = NUM_VERTS * Vertex3::NUM_FLOATS,
                NUM_FLOATS_FOR_NORMALS = NUM_VERTS * Vertex3::NUM_FLOATS,
                NUM_FLOATS_FOR_COLORS = NUM_VERTS * Color::NUM_FLOATS,
            };

            void set( const Vertex3* verts, const Vertex3* norms, const Color color )
            {
                for ( int vIdx = 0; vIdx < Cuboid::NUM_VERTS; vIdx++ )
                {
                    int vertexOffset = vIdx * 3;
                    m_vertices[vertexOffset] = verts[vIdx].x;
                    m_vertices[vertexOffset + 1] = verts[vIdx].y;
                    m_vertices[vertexOffset + 2] = verts[vIdx].z;

                    int normalOffset = vIdx * 3;
                    m_normals[normalOffset] = norms[vIdx].x;
                    m_normals[normalOffset + 1] = norms[vIdx].y;
                    m_normals[normalOffset + 2] = norms[vIdx].z;

                    int colorOffset = vIdx * 4;
                    m_colors[colorOffset] = color.r;
                    m_colors[colorOffset + 1] = color.g;
                    m_colors[colorOffset + 2] = color.b;
                    m_colors[colorOffset + 3] = color.a;
                }
            }

            void reset()
            {
                for ( int fltIdx = 0; fltIdx < Cuboid::NUM_FLOATS_FOR_VERTICES; fltIdx++ )
                {
                    m_vertices[fltIdx] = 0.f;
                }

                for ( int fltIdx = 0; fltIdx < Cuboid::NUM_FLOATS_FOR_NORMALS; fltIdx++ )
                {
                    m_normals[fltIdx] = 0.f;
                }

                for ( int fltIdx = 0; fltIdx < Cuboid::NUM_FLOATS_FOR_COLORS; fltIdx++ )
                {
                    m_colors[fltIdx] = 0.f;
                }
            }

            float* getVerts()
            {
                return m_vertices;
            }

            float* getNormals()
            {
                return m_normals;
            }

            float* getColors()
            {
                return m_colors;
            }

            // Model matrix local->world
            glm::mat4 m_model;

            // Vertices in cube-local space
            float m_vertices[Cuboid::NUM_FLOATS_FOR_VERTICES];

            // Unnormalized normals from cube-face
            float m_normals[Cuboid::NUM_FLOATS_FOR_NORMALS];

            // Color per vertex, r, g, b, a
            float m_colors[Cuboid::NUM_FLOATS_FOR_COLORS];
        };
 
        void create();
        int writeBufferCuboid( const Vertex3* trisAsVerts, const Vertex3* normalsPerVert, const glm::mat4& model, const Color color );
        void clearBufferCuboid( int index );
        void expandBuffer();
        void render( const glm::mat4& projection, const glm::mat4& view, const LightSource& lightSource, const glm::vec3 cameraPos );
        void setModel( int index, const glm::mat4& model );

        Shader* m_shader;

        GLuint m_vao;
        GLuint m_vbo;

        ArrayFreeList<Cuboid>* m_cuboids;

        int m_numCuboids;

        int m_nextFreeCuboid;
    };

    // Buffer generalized for convex /w variable # of vertices

    // Buffer generalized for meshes
    // Use a best-fit allocation for freelist
    struct DisplayGeometries
    {

    };

    // TODO: Order these members by data size?
    GLFWwindow* m_window;
    glm::vec3 m_backColor;

    Camera* m_camera;
    glm::mat4 m_projection;
    glm::mat4 m_view;

    DisplayLines m_displayLines;
    DisplayCuboids m_displayCuboids;

    LightSource m_lightSource;
};
