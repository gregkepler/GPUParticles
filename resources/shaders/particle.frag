uniform sampler2D spriteTexture;

varying vec2 index;

void main()
{
	gl_FragColor = texture2D(spriteTexture, gl_PointCoord) * vec4(index.x, index.y, 0, 1);
}