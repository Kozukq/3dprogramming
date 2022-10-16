function compile_shaders(gl, vsGLSL, fsGLSL) {

	const prg = gl.createProgram()
	const vertexShader = gl.createShader(gl.VERTEX_SHADER);
	const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);

	gl.shaderSource(vertexShader,vsGLSL);
	gl.shaderSource(fragmentShader,fsGLSL);

	gl.compileShader(vertexShader);
	if (!gl.getShaderParameter(vertexShader,gl.COMPILE_STATUS)) {
		throw new Error(gl.getShaderInfoLog(vertexShader))
	};

	gl.compileShader(fragmentShader);
	if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
		throw new Error(gl.getShaderInfoLog(fragmentShader))
	};	

	gl.attachShader(prg,vertexShader);
	gl.attachShader(prg,fragmentShader);

	gl.linkProgram(prg);
	if (!gl.getProgramParameter(prg,gl.LINK_STATUS)) {
		throw new Error(gl.getProgramInfoLog(prg))
	};	

	return prg;
}

function set_buffers(gl, vertices, indices) {

	const vertexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER,vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER,vertices,gl.STATIC_DRAW);

	const indexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,indices,gl.STATIC_DRAW);

	return { vertexBuffer, indexBuffer };
}

function update_buffers(gl, vertexBuffer, vertices, indexBuffer, indices) {

	gl.bindBuffer(gl.ARRAY_BUFFER,vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER,vertices,gl.STATIC_DRAW);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,indices,gl.STATIC_DRAW);
}