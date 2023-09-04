const fs = require("fs");

const filebase = process.argv[2];
const imageName = `${filebase}.data`

console.log(
  `imageName: ${imageName}`
);

const imageData = fs.readFileSync(imageName);

const tanka = [];
const tankb = [];
const tankc = [];

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
  tankc.push(b == 0 ? 0 : b - 1);
}

const createFile = (bytes, filename) => {
  let output = new Uint8Array(bytes);
  fs.writeFileSync(filename, output, "binary");
  console.log(`Generated file ${filename}`);
}

createFile(tanka, "TANKA.BIN")
createFile(tankb, "TANKB.BIN")
createFile(tankc, "TANKC.BIN")
