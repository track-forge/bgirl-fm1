// SPDX-License-Identifier: GPL-3.0-only
// A one-shot dissolve for a single-line text element. The real text stays in the DOM.
export async function dissolveOnEntry(element) {
  const motion = matchMedia('(prefers-reduced-motion: reduce)');
  if (!element || motion.matches) return;
  await document.fonts.ready;
  const observer = new IntersectionObserver(([entry]) => {
    if (!entry.isIntersecting) return;
    observer.disconnect();
    if (!motion.matches) dissolve(element, motion);
  }, {threshold:.6});
  observer.observe(element);
}

function dissolve(element, motion) {
  const style = getComputedStyle(element);
  const padding = 32;
  const width = element.offsetWidth, height = element.offsetHeight;
  if (!width || !height) return;
  const canvas = document.createElement('canvas');
  canvas.width = width + padding * 2;
  canvas.height = height + padding * 2;
  const ctx = canvas.getContext('2d');
  if (!ctx) return;
  ctx.font = `${style.fontWeight} ${style.fontSize} ${style.fontFamily}`;
  const label = style.textTransform === 'uppercase' ? element.textContent.trim().toUpperCase() : element.textContent.trim();
  const metrics = ctx.measureText(label);
  if (metrics.width > width + 1) return;
  const baseline = padding + (height + metrics.fontBoundingBoxAscent - metrics.fontBoundingBoxDescent) / 2;
  ctx.fillText(label, padding, baseline);
  const {data} = ctx.getImageData(0, 0, canvas.width, canvas.height);
  const particles = [];
  for (let y = padding; y < padding + height; y += 2) {
    for (let x = padding; x < padding + width; x += 2) {
      if (data[(y * canvas.width + x) * 4 + 3] < 80) continue;
      particles.push({x,y,dx:(Math.random()-.5)*56,dy:(Math.random()-.65)*44});
    }
  }
  if (!particles.length) return;
  // Let the intact title register before exchanging it for the particle layer.
  const text = document.createElement('span');
  text.textContent = element.textContent;
  element.replaceChildren(text);
  canvas.className = 'particle-dissolve';
  canvas.setAttribute('aria-hidden','true');
  canvas.style.inset = `-${padding}px auto auto -${padding}px`;
  element.classList.add('dissolving');
  element.append(canvas);
  const start = performance.now();
  function paint(now) {
    const progress = Math.min(1,(now-start)/2100);
    if (progress === 1 || motion.matches || element.offsetWidth !== width) {
      canvas.remove();
      element.classList.remove('dissolving');
      element.replaceChildren(...text.childNodes);
      return;
    }
    const phase = Math.max(0, Math.min(1, (progress - .22) / .70));
    const scatter = Math.sin(Math.PI * phase) ** 1.4;
    const mix = Math.min(1, scatter * 4);
    text.style.opacity = 1 - mix;
    canvas.style.opacity = mix;
    ctx.clearRect(0,0,canvas.width,canvas.height);
    ctx.fillStyle = style.color;
    for (const {x,y,dx,dy} of particles) {
      ctx.fillRect(x+dx*scatter,y+dy*scatter,2.4-scatter*.5,2.4-scatter*.5);
    }
    requestAnimationFrame(paint);
  }
  paint(start);
}
