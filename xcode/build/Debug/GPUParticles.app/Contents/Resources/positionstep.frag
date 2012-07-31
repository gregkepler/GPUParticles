uniform sampler2D velocitiesTexture;
uniform sampler2D positionsTexture;
uniform float deltaTime;

void main()
{
	vec4 posMap = texture2D(positionsTexture, gl_TexCoord[0].st);
	vec4 velMap = texture2D(velocitiesTexture, gl_TexCoord[0].st);
	gl_FragColor = posMap + velMap - vec4(0.50, 0.50, 0.0, 0.0);
}

