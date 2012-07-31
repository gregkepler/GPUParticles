uniform sampler2D positionMap;
uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform vec2 windowSize;

attribute vec2 index;

void main()
{
    gl_PointSize = 15.0;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec2 pos = texture2D(positionMap, index).xy * windowSize;
	gl_Position	= modelviewMatrix * projectionMatrix * vec4( gl_Vertex + vec4(pos.xy, 0.0, 0.0) );
}