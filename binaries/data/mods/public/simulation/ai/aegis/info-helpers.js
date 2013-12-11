/*
We track these entities only currently
	Units - 
	female_citizens //Support
	citizen_soldiers //Support, Army
	cavalry //Army
	infantry_melee //Support, Army
	infantry_ranged //Support, Army
	mechanical_seige //Army
	
	//Buildings -
	houses 
	fields
	farmstead
	barracks
	civil_centre 
	defense_tower
	market
	storehouse
	temple
*/

// the information state
var InfoState = function() {
	this.units = {
		female_citizen : 0.0, 
		citizen_soldier : 0.0,
		cavalry : 0.0,
		infantry_melee : 0.0,
		infantry_ranged : 0.0,
		mechanical_seige : 0.0
	};

	this.buildings = {
		house : 0.0,
		farmstead : 0.0,
		field : 0.0,
		barracks : 0.0,
		civil_centre : 0.0,
		market : 0.0,
		temple : 0.0,
		storehouse : 0.0,
		defense_tower : 0.0
	};
}

//the priorities for the different entities 
var InfoPriorities = function() {
	this.units = {
		female_citizen : 1.5,
		citizen_soldier : 2,
		cavalry : 3,
		infantry_melee : 2,
		infantry_ranged : 2,
		mechanical_seige : 4
	};
	// this[units] = this.units;

	this.buildings = {
		house : 5,
		farmstead : 3,
		field : 15,
		barracks : 20,
		civil_centre : 50,
		market : 12,
		temple : 15,
		storehouse : 15,
		defense_tower : 20
	};
}

//accumulator ctor
var InfoAccumulator = function(priorities) {
	this.accumulator = [];
	this.priorities = priorities;
	this.differences = [];
	this.accumulatorMaxLength = 10;
	for (var i = 0; i < this.accumulatorMaxLength - 1; ++i) {
		this.differences.push(new InfoState());
	}

	this.summary = null;
}

InfoAccumulator.prototype.add = function(state) {
	if (this.accumulator.length === this.accumulatorMaxLength) {
		this.accumulator.pop();		
	}
	this.accumulator.unshift(state);
}

InfoAccumulator.prototype.printAccumulator = function() {
	warn("-------------------------------------------------------------------------------------");
	for (i in this.accumulator) {
		warn("accumulator[" + i + "] -> " + uneval(this.accumulator[i]));
	}
	warn("-------------------------------------------------------------------------------------\n");
}

InfoAccumulator.prototype.updateWeightedDifferences = function() {
	this.summary = new InfoState();
	var length = this.accumulator.length - 1;
	var divisor;

	for (unit in this.priorities.units) {
		divisor = 1;
		for (var i = 0; i < length; ++i) {
			this.differences[i].units[unit] = (this.accumulator[i].units[unit] - this.accumulator[i+1].units[unit]) * this.priorities.units[unit];
			this.differences[i].units[unit] /= divisor;
			divisor *= 2;
			this.summary.units[unit] += this.differences[i].units[unit];
		}
	}
	for (building in this.priorities.buildings) {
		divisor = 1;
		for (var i = 0; i < length; ++i) {
			this.differences[i].buildings[building] = (this.accumulator[i].buildings[building] - this.accumulator[i+1].buildings[building]) * this.priorities.buildings[building];
			this.differences[i].buildings[building] /= divisor;
			divisor *= 2;
			this.summary.buildings[building] += this.differences[i].buildings[building];
		}
	}
}

InfoAccumulator.prototype.summarize = function() {
	this.updateWeightedDifferences();
}

InfoAccumulator.prototype.getSummary = function() {
	return this.summary;
}

InfoAccumulator.prototype.head = function() {
	var item = new InfoState();

	for (unit in item.units) {
		item.units[unit] = this.accumulator[0].units[unit] * this.priorities.units[unit];
	}

	for (building in item.buildings) {
		item.buildings[building] = this.accumulator[0].buildings[building] * this.priorities.buildings[building];
	}

	return item;
}

InfoAccumulator.prototype.compareLong = function() {
	var comparison = new InfoState();
	var first = this.accumulator[0];
	var last = this.accumulator[this.accumulator.length - 1];

	if (!first || first == last) {
		warn("Same!");
		return null;
	}

	for (unit in first.units) {
		comparison.units[unit] = (first.units[unit] - last.units[unit]) * this.priorities.units[unit];
	}

	for (building in first.buildings) {
		comparison.buildings[building] = (first.buildings[building] - last.buildings[building]) * this.priorities.buildings[building];
	}

	return comparison;
}

InfoAccumulator.prototype.compareMid = function() {
	var comparison = new InfoState();
	var first = this.accumulator[0];
	var last = this.accumulator[this.accumulator.length/2 | 0];

	if (!first || first == last) {
		warn("Same!");
		return null;
	}

	for (unit in first.units) {
		comparison.units[unit] = (first.units[unit] - last.units[unit]) * this.priorities.units[unit];
	}

	for (building in first.buildings) {
		comparison.buildings[building] = (first.buildings[building] - last.buildings[building]) * this.priorities.buildings[building];
	}

	return comparison;
}

InfoAccumulator.prototype.compareShort = function() {
	if (this.accumulator.length < 3) {
		warn("Less!");
		return null;
	}

	var comparison = new InfoState();
	var first = this.accumulator[0];
	var last = this.accumulator[2];

	for (unit in first.units) {
		comparison.units[unit] = (first.units[unit] - last.units[unit]) * this.priorities.units[unit];
	}

	for (building in first.buildings) {
		comparison.buildings[building] = (first.buildings[building] - last.buildings[building]) * this.priorities.buildings[building];
	}

	return comparison;
}