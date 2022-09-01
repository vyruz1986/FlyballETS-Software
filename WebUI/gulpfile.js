/*

ESP32 file system builder

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// -----------------------------------------------------------------------------
// File system builder
// -----------------------------------------------------------------------------

const fs = require("fs");
const gulp = require("gulp");
const htmlmin = require("gulp-htmlmin");
const gzip = require("gulp-gzip");
const inline = require("gulp-inline");
const inlineImages = require('gulp-css-base64');
const cleancss = require('gulp-clean-css');
const favicon = require("gulp-base64-favicon");
const htmllint = require("gulp-htmllint");
const c = require('ansi-colors');
const uglify = require("gulp-uglify");

const sourceFolder = "dist/";
const staticFolder = sourceFolder;
const outputFolder = "../include/";
//const outputFolder = "outputtest/";

String.prototype.replaceAll = function (search, replacement) {
   var target = this;
   return target.split(search).join(replacement);
};

gulp.task("toHeader", function (done) {
   var filename = "index.html.gz";
   var source = staticFolder + filename;
   var destination = outputFolder + filename + ".h";
   var safename = filename.replaceAll(".", "_");
   if (!fs.existsSync(outputFolder)) {
      fs.mkdirSync(outputFolder);
   }

   var wstream = fs.createWriteStream(destination);
   wstream.on("error", function (err) {
      console.log(err);
   });

   var data = fs.readFileSync(source);

   wstream.write("#define " + safename + "_len " + data.length + "\n");
   wstream.write("const uint8_t " + safename + "[] PROGMEM = {");

   for (i = 0; i < data.length; i++) {
      if (i % 20 == 0) wstream.write("\n");
      wstream.write("0x" + ("00" + data[i].toString(16)).slice(-2));
      if (i < data.length - 1) wstream.write(",");
   }

   wstream.write("\n};");
   wstream.end();
   done();
});

function htmllintReporter(filepath, issues) {
   if (issues.length > 0) {
      issues.forEach(function (issue) {
         console.log(c.cyan('[gulp-htmllint] ') +
            c.white(filepath + " [" + issue.line + "," + issue.column + "]: ") +
            c.colors.red("(" + issue.code + ") " + issue.msg)
         );
      });
      process.exitCode = 1;
   }
}

gulp.task("buildfs_inline", function () {
   return gulp.src(sourceFolder + "*.html") 
   .pipe(favicon())
      .pipe(
         inline({
            base: sourceFolder,
            js: [uglify],
            css: [cleancss, inlineImages],
            disabledTypes: ["svg", "img"],
         })
      )
      .pipe(
         htmllint(
            {
               failOnError: true,
               rules: {
                  "id-class-style": false,
                  "label-req-for": false,
                  "line-end-style": false,
                  "indent-width": false,
               },
            },
            htmllintReporter
         )
      )
      .pipe(
         htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true,
         })
      )
      .pipe(gzip({ gzipOptions: { level: 9 } }))
      .pipe(gulp.dest(staticFolder));
});

gulp.task("default", gulp.series("buildfs_inline", "toHeader"));
