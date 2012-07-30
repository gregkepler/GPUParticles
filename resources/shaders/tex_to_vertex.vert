uniform sampler2D positionMap;

varying vec2 index;

void main()
{
    gl_PointSize = 15.0;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	index = gl_TexCoord[0].st;
	vec4 pos = texture2D(positionMap, index) * 2.0 - vec4(1, 1, 0, 0);
	gl_Position	= pos * 2.0;
}