#pragma once

class BrowserWindow;

void session_save(const BrowserWindow& window);
void session_restore(BrowserWindow& window);
