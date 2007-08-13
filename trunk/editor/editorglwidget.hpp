
/*
   Copyright (C) 2007 by David White <dave.net>
   Copyright (C) 2007 by Isaac Clerencia <isaac@warp.es>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITORGLWIDGET_HPP_INCLUDED
#define EDITORGLWIDGET_HPP_INCLUDED

#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>

#include "../camera.hpp"
#include "../gamemap.hpp"

class EditorGLWidget : public QGLWidget
{
	Q_OBJECT

	public:
		EditorGLWidget(QWidget *parent = 0);
		void setMap(hex::gamemap *map);
		void setCamera(hex::camera *camera);

	protected:
		void initializeGL();
		void resizeGL(int w, int h);
		void paintGL();

		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void keyReleaseEvent(QKeyEvent *event);
		void mouseMoveEvent(QMouseEvent *event);

	protected slots:
		void checkKeys();

	private:
		hex::gamemap *map_;
		hex::camera *camera_;
		bool show_grid_;
		int radius_;
		int mousex_;
		int mousey_;
		QTimer timer_;
		QMap<int,bool> keys_;
};

#endif