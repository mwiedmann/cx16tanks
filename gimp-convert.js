const fs = require("fs");

const filebase = process.argv[2];
const imageName = `${filebase}.data`
const imageOutputFilename = `${filebase.toUpperCase()}.BIN`

console.log(
  `imageName: ${imageName}`
);

const imageData = fs.readFileSync(imageName);

const tanka = [];
const tankb = [];
let i;

for (i = 0; i<imageData.length; i+=3) {
  b=0
  if (imageData[i+1]>0) {
    b = 5
  } else if (imageData[i+2]>0) {
    b = 14
  }
  tanka.push(b);
  tankb.push(b == 0 ? 0 : b + 5);
}

output = new Uint8Array(tanka);
fs.writeFileSync("TANKA.BIN", output, "binary");

output = new Uint8Array(tankb);
fs.writeFileSync("TANKB.BIN", output, "binary");

console.log(`Generated file ${imageOutputFilename}`);
