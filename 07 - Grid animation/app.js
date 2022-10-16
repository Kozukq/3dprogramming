"use strict";

window.onload = main;

function main() {

	const canvas = document.querySelector('canvas');
	const gl = canvas.getContext('webgl');

	canvas.height = 600;
	canvas.width = 600;

	const vsGLSL = `
	attribute vec4 a_position;

	void main() {
	    gl_Position = a_position;
	}
	`;

	const fsGLSL = `
	precision highp float;

	void main() {
	    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	}
	`;

	const prg = compile_shaders(gl,vsGLSL,fsGLSL);

	const positionLoc = gl.getAttribLocation(prg, 'a_position');

	// var { vertices, indices, row_length, column_length } = generate_grid(0.10,0.10);
	var grid = generate_grid(0.1,0.1);

	update_grid(grid.vertices,50);

	var { vertexBuffer, indexBuffer } = set_buffers(gl,grid.vertices,grid.indices);

	gl.viewport(-275, -275, gl.canvas.width * 2, gl.canvas.height * 2);
	gl.enable(gl.DEPTH_TEST);
	gl.enable(gl.CULL_FACE);

	gl.clearColor(50/255,50/255,50/255,1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	gl.useProgram(prg);

	gl.enableVertexAttribArray(positionLoc);
	gl.bindBuffer(gl.ARRAY_BUFFER,vertexBuffer);
	gl.vertexAttribPointer(positionLoc,3,gl.FLOAT,false,0,0);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,indexBuffer);

	gl.drawElements(gl.LINES,grid.indices.length,gl.UNSIGNED_SHORT,0);

	var then = 0;

	requestAnimationFrame(render);

	function render(now) {

	    now *= 0.5; // convert to seconds

	    const deltaTime = now - then;

	    then = now;

	    update_grid(grid.vertices,then);

		update_buffers(gl,vertexBuffer,grid.vertices,indexBuffer,grid.indices);

		gl.viewport(-275, -275, gl.canvas.width * 2, gl.canvas.height * 2);
		gl.enable(gl.DEPTH_TEST); 
		gl.enable(gl.CULL_FACE);

		gl.clearColor(50/255,50/255,50/255,1.0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.useProgram(prg);

		gl.enableVertexAttribArray(positionLoc);
		gl.bindBuffer(gl.ARRAY_BUFFER,vertexBuffer);
		gl.vertexAttribPointer(positionLoc,3,gl.FLOAT,false,0,0);
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,indexBuffer);

		gl.drawElements(gl.LINES,grid.indices.length,gl.UNSIGNED_SHORT,0);

	    requestAnimationFrame(render);
	}
}

function generate_grid(row_distance, column_distance) {

	let nb_rows = Math.floor(1 / row_distance);
	let nb_columns = Math.floor(1 / column_distance);

	var vertices = new Float32Array(nb_rows * nb_columns * 3);

	let nb_row_indices = 2 * nb_rows * nb_columns - (2 * nb_columns);
	let nb_column_indices = 2 * nb_rows * nb_columns - (2 * nb_rows);

	var indices = new Uint16Array(nb_row_indices + nb_column_indices);

	let k = 0;

	for(let i = -0.5; i < 0.5; i += column_distance) {

		for(let j = -0.5; j < 0.5; j += row_distance) {

			vertices[k++] = i;
			vertices[k++] = j;
			vertices[k++] = 0.0;
		}
	}

	let nb_row_indices_per_row = nb_row_indices / nb_rows;
	let nb_column_indices_per_column = nb_column_indices / nb_columns;

	let i = 0
	let global_count = 0;

	// Calcul des indices des sommets des colonnes
	for(; i < nb_column_indices; i++) {

		// L'indice courant est le début d'une colonne
		if( i % nb_column_indices_per_column == 0 ) {

			indices[i] = global_count * nb_columns;

			global_count++; 
		}

		// L'indice courant est la fin d'une colonne
		else if( i % (nb_column_indices_per_column) == nb_column_indices_per_column - 1 ) {

			indices[i] = global_count * nb_columns - 1;
		}

		else {

			// L'indice précédent est le début d'une colonne
			if((i - 1) % nb_column_indices_per_column == 0) {

				indices[i] = indices[i-1] + 1;
			}

			// Les valeurs des deux indices précédents sont similaires
			else if(indices[i-1] == indices[i-2]) {

				indices[i] = indices[i-1] + 1;
			}

			else {

				indices[i] = indices[i-1];
			}
		}
	}

	let inner_count = 0;
	global_count = 0;

	// Calcul des indices des sommets des lignes
	for(; i < indices.length ; i++) {

		// L'indice courant est le début d'une ligne
		if( i % nb_row_indices_per_row == 0 ) {

			inner_count = 0;

			global_count++;
		}

		// Les valeurs des deux indices précédents sont similaires
		if(indices[i-1] == indices[i-2]) {

			inner_count++;
		}

		else if( (i - 1) % nb_row_indices_per_row == 0) {

			inner_count++;
		}

		indices[i] = global_count + inner_count * nb_rows - 1;
	}

	return { vertices : vertices,
			 indices : indices, 
			 row_length : nb_row_indices_per_row, 
			 column_length : nb_column_indices_per_column };
}

function update_grid(vertices, t) {

	for(let i = 0; i < vertices.length; i += 3) {

		if(i % 2 == 0) {

			vertices[i] += 0.0015 * Math.sin(vertices[i] + t / 100) / 2;
			vertices[i+1] += 0.002 * Math.cos(vertices[i+1] + t / 100) / 2;
		}
		else {

			vertices[i] -= 0.0015 * Math.sin(vertices[i] + t / 100) / 2;
			vertices[i+1] -= 0.002 * Math.cos(vertices[i+1] + t / 100) / 2;
		}
	}
}