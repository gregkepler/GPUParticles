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

#define SIDE 150

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

	float mElapsedTime;
    float* mPoints;
    float* mIndices;
    int mMaxParticles;
	int mDrawCycle;
};

void GPUParticlesApp::prepareSettings( Settings* settings)
{
	settings->setWindowSize( 1920, 1080 );
}

void GPUParticlesApp::setup()
{
	mDrawCycle = 0;
	mElapsedTime = 0.0;

	// Initialize positions and velocities textures
	int h = SIDE;
	int w = SIDE;
	mRect = Rectf(0.0,0.0,(float)w,(float)h);
	Area area( 0, 0, w, h );
    
    mMaxParticles = SIDE * SIDE;
		
	// Initialize the framebuffer objects
	gl::Fbo::Format fboTexFormat;
	fboTexFormat.setColorInternalFormat(GL_RGBA);
	
	gl::Texture::Format texFormat;
	texFormat.setInternalFormat(GL_RGBA);

	gl::setMatricesWindow(this->getWindowSize(), false);
    mFbos.reserve(4);
	for(int i = 0; i < 4; i++)
	{
		Surface32f surface(w, h, false);
		Surface32f::Iter iter = surface.getIter( area );
		while( iter.line() ) {
			while( iter.pixel() ) {
				if (i == 0 || i == 1)
				{
					// Velocities
					iter.r() = 0.5f + Rand::randFloat(-0.01f, 0.01f);
					iter.g() = 0.5f + Rand::randFloat(-0.01f, 0.01f);
					iter.b() = 0.0f;
					iter.a() = 1.0f;
				}
				else
				{
					// Positions
					iter.r() = 0.5f;
					iter.g() = 0.5f;
					iter.b() = 0.0f;
					iter.a() = 1.0f;
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
		mFbos[i] = fbo;
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
		loadResource( "particle.frag",			RES_FRAG_PARTICLES,			"GLSL" ));

	/*vector<Vec2f> texCoords;
	vector<Vec3f> vCoords, normCoords;
	vector<uint32_t> indices;

	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
    
	// Initialize VBO for point sprite vertices
	int totalVertices = SIDE * SIDE;

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
	mVboMesh.bufferNormals( normCoords );*/
    
    // Vertex array
    int numVertices = this->mMaxParticles * 2;
    mPoints = (float*) calloc(sizeof(float), numVertices);
    /*for(int i = 0; i < numVertices-1; i+=2) {
        mPoints[i] = 100.0f;
        mPoints[i+1] = 200.0f;
    }*/
    
    mIndices = (float*) calloc(sizeof(float), numVertices);
    int i = 0;
	for( int x = 0; x < SIDE; ++x ) {
		for( int y = 0; y < SIDE; ++y ) {
            mIndices[i] = (float)x / (float)SIDE;
            mIndices[i+1] = (float)y / (float)SIDE;
            console() << "[ " << x << ", " << y << " ]" << std::endl;
            i+=2;
        }
    }

    

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

	float deltaTime = this->getElapsedSeconds() - mElapsedTime;
	mElapsedTime = this->getElapsedSeconds();

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
		mVelocityShader.uniform("elapsedTime", mElapsedTime);
		gl::drawSolidRect(mRect);
		mVelocityShader.unbind();
		fboVelocitySource->getTexture().unbind();
	fboVelocityTarget->unbindFramebuffer();

	// DEBUG: To draw the velocities texture
	//gl::setMatricesWindow(this->getWindowSize(), true);
	//gl::draw(fboVelocityTarget->getTexture());
    //fboVelocityTarget->getTexture().unbind();

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

	// DEBUG: To draw the positions texture
	gl::setMatricesWindow(this->getWindowSize(), true);
	gl::draw(fboPositionTarget->getTexture());
    
	// draw particles with updated position data
	gl::setMatricesWindow(this->getWindowSize(), true);
    
	glEnable(GL_BLEND);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	mParticleShader.bind();
	fboPositionTarget->getTexture().bind(0);
	mSpriteTexture.bind(1);
    
	mParticleShader.uniform("positionMap", 0);
	mParticleShader.uniform("spriteTexture", 1);
	mParticleShader.uniform("modelviewMatrix", gl::getModelView());
	mParticleShader.uniform("projectionMatrix", gl::getProjection());
    mParticleShader.uniform("windowSize", Vec2f(this->getWindowSize().x, this->getWindowSize().y));
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, mPoints);
    glDrawArrays(GL_POINTS, 0, mMaxParticles * 2.0);
    
    GLint loc = mParticleShader.getAttribLocation("index");
    glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 2, mIndices);
    
	mSpriteTexture.unbind();
	fboPositionTarget->getTexture().unbind();
	mParticleShader.unbind();
    
    glDisable(GL_VERTEX_ATTRIB_ARRAY_POINTER);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_BLEND);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_TEXTURE_2D);
    
}

CINDER_APP_BASIC( GPUParticlesApp, RendererGl )
