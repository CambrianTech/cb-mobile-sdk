/**
 * @author alteredq / http://alteredqualia.com/
 */

 THREE.CBIntegratorShader = {

	uniforms: {

		"tDiffuse": { value: null },
		"tMask": { value: null },
		"opacity":  { value: 1.0 }

	},

	vertexShader: [

		"varying vec2 vUv;",
        "uniform float aspect;",

		"void main() {",
			"vUv = uv;",
			"gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0);",

		"}"

	].join( "\n" ),

	fragmentShader: [

		"uniform float opacity;",
		"uniform sampler2D tDiffuse;",
		"uniform sampler2D tMask;",
        "varying vec2 vUv;",

		"void main() {",
			"vec4 texel = texture2D( tDiffuse, vUv );",
			"vec4 maskValue = texture2D( tMask, vUv );",
			"gl_FragColor = texel;",
			"gl_FragColor.a = 1.0 - opacity * maskValue.r;",
		"}"

	].join( "\n" )

};


THREE.CBIntegratorPass = function ( sceneCamera, map, opacity ) {

	THREE.Pass.call( this );

	var shader = THREE.CBIntegratorShader;

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

THREE.CBIntegratorPass.prototype = Object.assign( Object.create( THREE.Pass.prototype ), {

	constructor: THREE.CBIntegratorPass,

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

		this.uniforms[ "opacity" ].value = this.opacity;
		this.uniforms[ "tDiffuse" ].value = this.map;
		this.uniforms[ "tMask" ].value = this.maskMap;
		this.material.transparent = true;

		renderer.render( this.scene, this.camera, this.renderToScreen ? null : readBuffer, this.clear );
		renderer.autoClear = oldAutoClear;
	}

} );
