#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 2000
#define HEIGHT 768

class BRU12App : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void update() override;
    void draw() override;

    BRU12App();

    CameraPersp camera;
    gl::BatchRef rectBatch;
    gl::GlslProgRef	glslProg;

    vector<vec3> vertices;
    vector<uint32_t> indices;
    vector<Color> colors;

    gl::VboMeshRef mesh;
    gl::BatchRef meshBatch;
};

BRU12App::BRU12App() : camera(WIDTH, HEIGHT, 90, 0.0001, 10000) {}

void BRU12App::setup() {
    camera.lookAt( vec3( 2, 1, 3 ), vec3( 0 ) );

    auto glsl = gl::GlslProg::Format()
        .vertex(loadAsset("../Resources/vert.glsl"))
        .fragment(loadAsset("../Resources/frag.glsl"));
    glslProg = gl::GlslProg::create( glsl );

    auto rect = geom::Rect().colors(Color(1, 0, 0),
                                    Color(0.2, 0.5, 1),
                                    Color(0.9, 0.1, 0.5),
                                    Color(1, 0.2, 0.9));
    rectBatch = gl::Batch::create(rect, glslProg);

    vertices = {
        vec3(0, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 1, 1),
        vec3(3, 20, 3)
    };
    indices = {
        0, 1, 2,
        2, 1, 3,
        0, 1, 3,
        0, 2, 3
    };
    colors = {
        Color(1, 0, 0),
        Color(1, 1, 0),
        Color(1, 0, 1),
        Color(1, 1, 1)
    };

    gl::VboRef vbo = gl::Vbo::create(GL_ARRAY_BUFFER, indices, GL_DYNAMIC_DRAW);

    vector<gl::VboMesh::Layout> layout = {
        gl::VboMesh::Layout().usage(GL_DYNAMIC_DRAW).attrib(geom::Attrib::POSITION, 3),
        gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::COLOR, 3)
    };

    mesh = gl::VboMesh::create(sizeof(vec3) * vertices.size(), GL_TRIANGLES, layout//,
//                               indices.size(), GL_UNSIGNED_INT, vbo
                               );
    mesh->bufferAttrib(geom::Attrib::POSITION, sizeof(vec3) * vertices.size(), vertices.data());
    mesh->bufferAttrib(geom::Attrib::COLOR, sizeof(Color) * colors.size(), colors.data());
//    mesh->bufferIndices(indices.size(), indices.data());
    meshBatch = gl::Batch::create(mesh, glslProg);

    gl::enableDepthWrite();
    gl::enableDepthRead();

    GLint numAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttribs);
    console() << "Number of vertex attributes: " << numAttribs << endl;
}

void BRU12App::mouseDown(MouseEvent event) {
}

void BRU12App::update() {
}

void BRU12App::draw() {
    gl::clear(Color(0.2f, 0.2f, 0.2f));
    gl::setMatrices(camera);
//    rectBatch->draw();
    meshBatch->draw();
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
