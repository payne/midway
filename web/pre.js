Module.midwayKeyQueue = [];

document.addEventListener('keydown', (event) => {
  const key = event.key;
  let consumed = false;
  if (key === 'Shift' || key === 'Control' || key === 'Meta' || key === 'Alt') {
    return;
  }

  if (key === 'Enter') {
    Module.midwayKeyQueue.push(10);
    consumed = true;
  } else if (key === 'Backspace') {
    Module.midwayKeyQueue.push(8);
    consumed = true;
  } else if (key.length === 1) {
    Module.midwayKeyQueue.push(key.charCodeAt(0));
    consumed = true;
  }

  if (consumed) {
    event.preventDefault();
  }
});
