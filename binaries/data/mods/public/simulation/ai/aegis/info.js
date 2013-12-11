/*
Stores game information
What we do is this - 
	count the number of entities every few seconds. 
	detect a change in the number of entitites
	each entitiy has a priority. Depending on how bad the change was, take action

How this is structured -
	we do a lot of CPU intensive tasks. Since we know that each player gets a turn to play once in 2 seconds, we'll split this calculation of game information into two steps
	1. Analysis
	2. Process
	Both of these steps peform partial actions over 3 three player turns (that's around 6 seconds). This reduces the load of AI in every turn and also we don't need to check for stuff every 2 seconds!
	Again, analysis and parallel run parallely. But both complete their job every 3 player turns.

What we have -
	InfoState class (declared in info-helper.js). An object of this class stores the game informtion state at one point.
	The InfoAccumulator accumulates a list of the 10 recent information states. (we've currently chosen 10, although it might seem big)
		This accumulator provides a "summarize()" function to get the summary of these 10 recent information states.
		Based on the value of the summary for a unit/structure, the processor will take actions
	So, The analysis is distributed over 3 turns. 
		In the first turn, we get unit information
		In the second turn, we get structure information
		In the third turn, we analyze this information

How the analysis is performed -
	The summary obtained is a weighted difference, calculated as follows -
		We take successive differences of the 10 recent information states.
		We then add these multiplied to a weightage of 1/(2^i) where i is the index, starting from zero.
		This says to consider recent changes more than changes in the past. (Don't live in the past!)
		so its something like 
			summary = a[0]*1 + a[1]*0.5 + a[2]*0.25 + ...

A lot of processing will be done too, so even that can be split over 3 turns.
Remember, the accumulator contains what analyzed in full, most recently. 
There will be another information state which is still being analyzed (details are still being queried and summary being calculated). This state will not be present in the accumulator.
*/

var Info = function () {
	this.curState = new InfoState();
	this.priorities = new InfoPriorities();
	this.states = new InfoAccumulator(this.priorities);

	this.analysisTurn = 0;
	this.processTurn = 0;
	this.processingDisaster = false;
}

//count the units in the game
Info.prototype.countUnits = function(gameState) {
	this.curState.units["female_citizen"] = gameState.getOwnEntities().filter(Filters.byClass("Support")).length;
	this.curState.units["infantry_melee"] = gameState.getOwnEntities().filter(Filters.byClassesAnd(["Infantry","Melee"])).length;
	this.curState.units["infantry_ranged"] = gameState.getOwnEntities().filter(Filters.byClassesAnd(["Infantry","Ranged"])).length;
	this.curState.units["citizen_soldier"] = this.curState.units.infantry_melee + this.curState.units.infantry_ranged;
	this.curState.units["cavalry"] = gameState.getOwnEntities().filter(Filters.byClass("Cavalry")).length;
	this.curState.units["mechanical_seige"] = gameState.getOwnEntities().filter(Filters.byClass("Siege")).length;
}

//count the buildings in the game
Info.prototype.countBuildings = function(gameState) {
	this.curState.buildings["house"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_house"), true);
	this.curState.buildings["farmstead"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_farmstead"), true);
	this.curState.buildings["field"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_field"), true);
	this.curState.buildings["barracks"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_barracks"), true);
	this.curState.buildings["civil_centre"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_civil_centre"), true);
	this.curState.buildings["market"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_market"), true);
	this.curState.buildings["temple"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_temple"), true);
	this.curState.buildings["storehouse"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_storehouse"), true);
	this.curState.buildings["defense_tower"] = gameState.countEntitiesByType(gameState.applyCiv("structures/{civ}_defense_tower"), true);
}

//analyze the changes in the information state
//currently just adds the information state to the accumulator and updates the summary
Info.prototype.analyzeChanges = function(gameState)  {
	this.states.add(this.curState);
	this.states.summarize();
}

// the different analysis turns
Info.prototype.analyze = function(gameState) {
	if (this.analysisTurn == 0) {
		this.countUnits(gameState);
	} else if (this.analysisTurn == 1) {
		this.countBuildings(gameState);
	} else {
		// warn(uneval(this.curState.units));
		// warn(uneval(this.curState.buildings));
		this.analyzeChanges(gameState);
		this.curState = new InfoState();
		this.analysisTurn = -1;
	}
	++this.analysisTurn;
}

//the different process turns
Info.prototype.process = function(gameState) {
	if (this.processTurn == 0) {
		//check for support damage
		var summary = this.states.getSummary();
		if (summary !== null) {
			if (summary.units.female_citizen < -2) {
				if (!this.processingDisaster) {
					this.processingDisaster = true;
					var HQ = gameState.ai.HQ;
					if (HQ.upcomingAttacks.CityAttack.length > 0) {
						// HQ.upcomingAttacks.CityAttack.pop()
						gameState.ai.queueManager.pauseQueue("plan_0", true);
						gameState.ai.queueManager.queues["plan_0"].empty();
						HQ.upcomingAttacks.CityAttack[0].SetPaused(gameState, true);
					}
				}
			} else {
				this.processingDisaster = false;
			}
		}
	} else if (this.processTurn == 1) {
		//check for military damage

	} else {
		this.processTurn = -1;
	}

	++this.processTurn;
}

//update game information
Info.prototype.update = function (gameState) {
	this.process(gameState);
	this.analyze(gameState);
}