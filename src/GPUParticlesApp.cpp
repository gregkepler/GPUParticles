#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/Surface.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Area.h"
#include "cinder/Rect.h"
#include "cinder/ImageIo.h"

#include "Resources.h"

#define SIDE 300

using namespace ci;
using namespace ci::app;
using namespace std;

class GPUParticlesApp : public AppBasic {
public:
	void setup();
	void mouseDown( MouseEvent event );	
	void prepareSettings( Settings* settings);
	void update();
	void draw();
private:
	gl::Texture mSpriteTexture;

	std::vector<gl::Fbo*> mFbos;

	gl::VboMesh mVboMesh;

	gl::GlslProg mVelocityShader;
	gl::GlslProg mParticleShader;
	gl::GlslProg mPositionShader;
	Rectf mRect;

	int mDrawCycle;
};

void GPUParticlesApp::prepareSettings( Settings* settings)
{
	settings->setWindowSize( 1920, 1080 );
}

void GPUParticlesApp::setup()
{
	mDrawCycle = 0;

	// Initialize positions and velocities textures
	int h = SIDE;
	int w = SIDE;
	mRect = Rectf(0.0,0.0,(float)w,(float)h);
	Area area( 0, 0, w, h );
		
	// Initialize the framebuffer objects
	gl::Fbo::Format fboTexFormat;
	fboTexFormat.setColorInternalFormat(GL_R32F);
	
	gl::Texture::Format texFormat;
	texFormat.setInternalFormat(GL_R32F);

	gl::setMatricesWindow(this->getWindowSize(), false);
	for(int i = 0; i < 4; i++)
	{
		Surface32f surface(w, h, false);
		Surface32f::Iter iter = surface.getIter( area );
		while( iter.line() ) {
			while( iter.pixel() ) {
				if (i == 0 || i == 1)
				{
					iter.r() = Rand::randFloat(-1.0,1.0);
					iter.g() = Rand::randFloat(-1.0,1.0);
					iter.b() = 0;
					iter.a() = 1;
				}
				else
				{
					iter.r() = 0;
					iter.g() = 0;
					iter.b() = 0;
					iter.a() = 1;
				}
			}
		}
		gl::Texture startTexture = gl::Texture(w, h, texFormat);
		startTexture.update(surface);

		gl::Fbo* fbo = new gl::Fbo(w, h, fboTexFormat);
		gl::clear( Color( 0, 0, 0 ) ); 
		fbo->bindFramebuffer();
		gl::draw(startTexture);
		fbo->unbindFramebuffer();
		mFbos.push_back(fbo);
	}

	// Load the shaders
	mVelocityShader = gl::GlslProg(
		loadResource( "passthru.vert",			RES_VERT_PASSTHRU,			"GLSL"),
		loadResource( "velocitystep.frag",		RES_FRAG_VELOCITYSTEP,		"GLSL" ));

	mPositionShader = gl::GlslProg(
		loadResource( "passthru.vert",			RES_VERT_PASSTHRU,			"GLSL"),
		loadResource( "positionstep.frag",		RES_FRAG_POSITIONSTEP,		"GLSL" ));

	mParticleShader = gl::GlslProg(
		loadResource( "tex_to_vertex.vert",		RES_VERT_TEX_TO_VERT,		"GLSL"),
		loadResource( "particles.frag",			RES_FRAG_PARTICLES,			"GLSL" ));

	// Initialize VBO for point sprite vertices
	int totalVertices = SIDE * SIDE;

	vector<Vec2f> texCoords;
	vector<Vec3f> vCoords, normCoords;
	vector<uint32_t> indices;

	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();

	mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS); // GL_LINES GL_QUADS
	for( int x = 0; x < SIDE; ++x ) {
		for( int y = 0; y < SIDE; ++y ) {	
			indices.push_back( x * SIDE + y );
			texCoords.push_back( Vec2f( x/(float)SIDE, y/(float)SIDE ) );
			vCoords.push_back( Vec3f( 0, 0, 0  ) );
			normCoords.push_back( Vec3f( 0.0f, 1.0f, 0.0f ));
		}
	}

	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
	mVboMesh.bufferPositions( vCoords );
	mVboMesh.bufferNormals( normCoords );

	// Point sprite texture
	mSpriteTexture = gl::Texture( loadImage ( loadResource( "pointsprite.png", RES_POINT_SPRITE, "IMAGE") ) );
	mSpriteTexture.setWrap( GL_REPEAT, GL_REPEAT );
	mSpriteTexture.setMinFilter( GL_NEAREST );
	mSpriteTexture.setMagFilter( GL_NEAREST );
}

void GPUParticlesApp::mouseDown( MouseEvent event )
{
}

void GPUParticlesApp::update()
{
}

void GPUParticlesApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	glEnable(GL_TEXTURE_2D);

	gl::setMatricesWindow(this->getWindowSize(), false);

	// Ping ponging of fbos
	if (++mDrawCycle >= 2) mDrawCycle = 0;
	gl::Fbo* fboVelocityTarget = mDrawCycle == 0 ? mFbos[0] : mFbos[1];
	gl::Fbo* fboVelocitySource = mDrawCycle == 0 ? mFbos[1] : mFbos[0];
	gl::Fbo* fboPositionTarget = mDrawCycle == 0 ? mFbos[2] : mFbos[3];
	gl::Fbo* fboPositionSource = mDrawCycle == 0 ? mFbos[3] : mFbos[2];

	fboVelocityTarget->bindFramebuffer();
		fboVelocitySource->getTexture().bind(0);
		mVelocityShader.bind();
		mVelocityShader.uniform("velocitiesTexture", 0);
		gl::drawSolidRect(mRect);
		mVelocityShader.unbind();
		fboVelocitySource->getTexture().unbind();
	fboVelocityTarget->unbindFramebuffer();

	// DEBUG: To draw the velocities texture
	//gl::setMatricesWindow(this->getWindowSize(), true);
	//gl::draw(fboPositionTarget->getTexture());
	// return;

	fboPositionTarget->bindFramebuffer();
		gl::clear( Color( 0, 0, 0) );
		mPositionShader.bind();
		fboPositionSource->getTexture().bind(0); // the current positions (not calculated upon yet)
		fboVelocityTarget->getTexture().bind(1); // the updated velocities (computed above)
		mPositionShader.uniform("positionsTexture", 0);
		mPositionShader.uniform("velocitiesTexture", 1);
		gl::drawSolidRect(mRect);
		mPositionShader.unbind();
		fboVelocityTarget->getTexture().unbind();
		fboPositionSource->getTexture().unbind();
	fboPositionTarget->unbindFramebuffer();

	// DEBUG: To draw the velocities texture
	gl::setMatricesWindow(this->getWindowSize(), true);
	fboPositionTarget->bindTexture(0, 0);
	gl::draw(fboPositionTarget->getTexture());
	fboPositionTarget->unbindTexture();

	
	gl::setMatricesWindow(this->getWindowSize(), true);

	// draw particles with updated position data
	glEnable(GL_BLEND);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	mParticleShader.bind();
	fboPositionTarget->getTexture().bind(0);
	mSpriteTexture.bind(1);

	mParticleShader.uniform("positionMap", 0);
	mParticleShader.uniform("spriteTexture", 1);

	gl::draw( mVboMesh );

	mSpriteTexture.unbind();
	fboPositionTarget->getTexture().unbind();
	mParticleShader.unbind();
	
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_BLEND);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_TEXTURE_2D);
}

CINDER_APP_BASIC( GPUParticlesApp, RendererGl )
