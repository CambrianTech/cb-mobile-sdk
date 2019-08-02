/**
 * @author alteredq / http://alteredqualia.com/
 */

 THREE.CBHighlightsShader = {

	uniforms: {

		"tBase": { value: null },
		"tHighlights": { value: null },
		"strength":  { value: 1.0 }

	},

	vertexShader: [

		"varying vec2 vUv;",
        "uniform float aspect;",

		"void main() {",
			"vUv = uv;",
			"gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0);",

		"}"

	].join( "\n" ),

	fragmentShader:

		`
		uniform float strength;
		uniform sampler2D tBase;
		uniform sampler2D tHighlights;
        varying vec2 vUv;

        vec3 rgb2hsv(vec3 c) {
		    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
		    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
		    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

		    float d = q.x - min(q.w, q.y);
		    float e = 1.0e-10;
		    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
		}

		vec3 hsv2rgb(vec3 c) {
    		vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    		vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    		return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
		}

		void main() {
			vec4 texel = texture2D( tBase, vUv );
			vec4 highlightsValue = texture2D( tHighlights, vUv );
			vec3 hsv = rgb2hsv(texel.rgb);
			float value = pow(2.0 * highlightsValue.r, 2.0);
			hsv.b = mix(hsv.b, value, strength);
			vec3 color = hsv2rgb(hsv);
			gl_FragColor = vec4(color, 1.0);
		}
		`
};


THREE.CBHighlightsPass = function ( sceneCamera, map, opacity ) {

	THREE.Pass.call( this );

	var shader = THREE.CBHighlightsShader;

	this.map = map;
	this.opacity = ( opacity !== undefined ) ? opacity : 1.0;

	this.uniforms = THREE.UniformsUtils.clone( shader.uniforms );

	this.material = new THREE.ShaderMaterial( {

		uniforms: this.uniforms,
		vertexShader: shader.vertexShader,
		fragmentShader: shader.fragmentShader,
		depthTest: false,
		depthWrite: false

	} );

	this.needsSwap = false;

	this.sceneCamera = sceneCamera;
	this.camera = new THREE.OrthographicCamera( - 1, 1, 1, - 1, 0, 1 );
	this.scene  = new THREE.Scene();

	this.quad = new THREE.Mesh( new THREE.PlaneBufferGeometry( 2, 2 ), null );
	this.quad.frustumCulled = false; // Avoid getting clipped
	this.scene.add( this.quad );

};

THREE.CBHighlightsPass.prototype = Object.assign( Object.create( THREE.Pass.prototype ), {

	constructor: THREE.CBHighlightsPass,

	render: function ( renderer, writeBuffer, readBuffer, delta, maskActive ) {

		var oldAutoClear = renderer.autoClear;
		var aspect = this.sceneCamera.aspect;
		renderer.autoClear = false;

		if(aspect > 1) {
		    this.quad.scale.x = 1.0;
            this.quad.scale.y = 1.0 * aspect;
        } else {
		    this.quad.scale.y = 1.0;
		    this.quad.scale.x = 1.0 / aspect;
        }

		this.quad.material = this.material;

		this.uniforms[ "strength" ].value = this.strength;
		this.uniforms[ "tBase" ].value = this.sourceTexture;
		this.uniforms[ "tHighlights" ].value = this.highlightsMap;
		this.material.transparent = true;

		renderer.render( this.scene, this.camera, this.renderToScreen ? null : readBuffer, this.clear );
		renderer.autoClear = oldAutoClear;
	}

} );
