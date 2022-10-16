const algoEnvConv = {demiPlan: 0, jarvis:1, graham:2};

class EnveloppeConvexe{
	constructor(lesPoints, a){
		this.algo = a ?? algoEnvConv.jarvis;
		this.points = lesPoints;
		this.setAlgo(this.algo);
	}

	getElements (){
		let ec = new Array();
		for (let i=0; i < this.envconv.length; i++){
			ec.push(this.points[this.envconv[i]]);
		}
		return ec;
	}

	setAlgo(idAlgo){
		this.algo = idAlgo;
		switch (this.algo){
			case algoEnvConv.demiPlan:
				this.envconv = this.algoDemiPlan(this.points);
				break;
			case algoEnvConv.jarvis:
				this.envconv = this.algoJarvis(this.points);
				break;
			case algoEnvConv.graham:
				this.envconv = this.algoGraham(this.points);
				break;	
			default:
				console.log("algo non défini => algo jarvis utilisé")
				this.envconv = this.algoDemiPlan(this.points);
				break;
		}
	}

	findMinIdx(points){
		let imin = 0;
		for(let i = 1; i < points.length; i++){
			if (points[i].y < points[imin].y)
				imin = i;
		}
		return imin;
	}

	determinant(v1,v2){
		return v1.x * v2.y - v1.y * v2.x;
	}

	detSign(v1,v2){
		let d = this.determinant(v1,v2);
		if (d > 0) return 1;
		if (d == 0) return 0;
		return -1;
	}

	vect(a,b){
		return{x:b.x-a.x,y:b.y - a.y}
	}

	tour(a,b,c){
		return this.detSign(this.vect(a,b), this.vect(a,c));
	}


	findFirst(i,j, points){
		let val;

		for(let k = 0; k < points.length; k++){
			if(i != k && j != k){
				val = k;
				break;
			} 
		}
		return val;
	}

	findNext(start, points){

		let val;

		if(start != 0) val = 0;
		else val = 1;

		for(let i = 0; i < points.length; i++){
			let sens = this.tour(this.points[start], this.points[val], this.points[i]);
			if(sens == -1)
				if(start != val && start != i && val != i){
					val = i;
				}
		}
		
		return val;
	}

	algoDemiPlan(points){

		// Initialisations
		console.log("algo d'intersection des demi-plans")
		let envconv = new Array();
		let n = points.length;
		let current, previous;
		let i,j,k;

		// Début
		for(i = 0; i < n; i++){
			for(j = i+1; j < n; j++){
				let mc = true;  // mc = meme côté
				let k = 0;

				do{
					if(k == 0){
						// Premier point
						let first = points[this.findFirst(i,j,points)];
						previous = this.tour(points[i],points[j], first);

					}

					if(k != i && k != j){
						current = this.tour(points[i],points[j],points[k]);

						if(current == 0){ // lance une exception
							console.log("Alignement");
						}
						else if ( current != previous){
							mc = false;
						}

					}

					k++;
				} while( k < n && mc)


				if( k == n && previous == current){
					if(current > 0) envconv.push(i,j);
					if(current < 0) envconv.push(j,i);
					if(current == 0) console.log("Alignement 2 ");
				}
			}
		}

		return envconv;
	}

	 algoJarvis(points){
		console.log("algo Jarvis")

		// initialisaiton
		let min = this.findMinIdx(points);
		let envconv = new Array();
		envconv.push(min);
		let courant;
		let previous = min;

		// début
		do{
			if(courant != null ) envconv.push(courant);
			courant = this.findNext(previous, points);
			envconv.push(courant);
			previous = courant;
		}while(courant != min)

		// résultat 
		return envconv;
	}

	algoGraham(points){
		let imin = this.findMinIdx(points);
		let ec = new Array();
		ec[0] = imin;

		//todo

		return ec;
	}
}