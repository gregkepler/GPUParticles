uniform sampler2D spriteTexture;

varying vec2 index;

void main()
{
	//gl_FragColor = vec4(index.x, index.y, 0, 1);
	gl_FragColor = vec4(1,0,0,1);
}