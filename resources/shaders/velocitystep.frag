uniform sampler2D velocitiesTexture;
uniform float elapsedTime;

void main()
{
	vec4 force = vec4(0,0,0,0);
	vec4 velMap = texture2D(velocitiesTexture, gl_TexCoord[0].st);
	gl_FragColor = velMap + force;
}