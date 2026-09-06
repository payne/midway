Module.midwayKeyQueue = [];

document.addEventListener('keydown', (event) => {
  const key = event.key;
  if (key === 'Shift' || key === 'Control' || key === 'Meta' || key === 'Alt') {
    return;
  }

  if (key === 'Enter') {
    Module.midwayKeyQueue.push(10);
  } else if (key === 'Backspace') {
    Module.midwayKeyQueue.push(8);
  } else if (key.length === 1) {
    Module.midwayKeyQueue.push(key.charCodeAt(0));
  }

  event.preventDefault();
});
