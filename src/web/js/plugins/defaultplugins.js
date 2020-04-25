/* both composer and designer updates the following projects in the local IndexedDB on startup */
/*var DefaultPlugins = [
	SimpleSynth,
	SimpleKick,
	SimpleDelay,
	SimpleDist,
	SimpleGain,
	SimpleEq,
	SimpleReverb,
	SimpleFilter,
	SimpleKarplus,
	WobbleSynth
];
*/
var DefaultPlugins2 = [
	"http://modulyzer.com/json/simplesynth.json",
	"http://modulyzer.com/json/simplekarplus.json",
	"http://modulyzer.com/json/wobblesynth.json",
	"http://modulyzer.com/json/simplekick.json",
	"http://modulyzer.com/json/simpledelay.json",
	"http://modulyzer.com/json/simplereverb.json"
];

/*
NOTE: refering online corsenabled plugins, cant load category or name without loading everything
TODO: give name, version, allow to reload into db based on version check
TODO: develop into manifest - json with json references? can refer plugin bundles by url, and save bundle urls in db!
TODO: include documetation as well: plugin browser w/parameter details and shit
*/