const fs = require("fs")
const input = JSON.parse(fs.readFileSync('tanks.ldtk'))

let output = new Uint8Array(input.levels[0].layerInstances[0].intGridCsv)
fs.writeFileSync('MAZE.BIN', output, "binary");
