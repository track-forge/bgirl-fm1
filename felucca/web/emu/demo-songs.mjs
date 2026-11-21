// SPDX-License-Identifier: GPL-3.0-only
import {parse} from './song-snapshot.mjs';

export async function loadTicker() {
  const response = await fetch(new URL('./demos/ticker.bgirl.json', import.meta.url));
  if (!response.ok) throw new Error(`Could not load Ticker (${response.status}).`);
  return parse(await response.text());
}
