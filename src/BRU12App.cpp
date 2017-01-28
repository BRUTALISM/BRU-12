#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BRU12App : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void BRU12App::setup()
{
}

void BRU12App::mouseDown( MouseEvent event )
{
}

void BRU12App::update()
{
}

void BRU12App::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( BRU12App, RendererGl )
