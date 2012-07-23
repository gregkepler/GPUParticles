uniform sampler2D positionMap;

varying vec2 index;

void main()
{
    gl_PointSize = 15.0;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	index = gl_TexCoord[0].st;
	gl_Position	= texture2D(positionMap, index);
}