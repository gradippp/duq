const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const DOCS_DIR = __dirname;
const BUILD_DIR = path.join(DOCS_DIR, 'build');
const GIT_INFO_FILE = path.join(BUILD_DIR, 'git_info.tex');

// 1. Ensure build directory and subdirectories exist
if (!fs.existsSync(BUILD_DIR)) {
    fs.mkdirSync(BUILD_DIR, { recursive: true });
}

// LaTeX \include{chapters/...} requires the 'chapters' directory to exist in the output-directory
const chaptersBuildDir = path.join(BUILD_DIR, 'chapters');
if (!fs.existsSync(chaptersBuildDir)) {
    fs.mkdirSync(chaptersBuildDir, { recursive: true });
}

// 2. Get versions and git commit hash
const pkg = JSON.parse(fs.readFileSync(path.join(DOCS_DIR, 'package.json'), 'utf8'));
const manualVersion = pkg.version;
let softwareVersion = process.argv[2] || '0.0.0';
let gitCommit = process.env.GIT_COMMIT;

if (!gitCommit || gitCommit === '""') {
    try {
        const git = spawnSync('git', ['rev-parse', '--short', 'HEAD'], { encoding: 'utf8' });
        if (git.status === 0) {
            gitCommit = git.stdout.trim();
        }
    } catch (e) {
        console.warn('Failed to get git commit hash via git command.');
    }
}

if (!gitCommit || gitCommit === '""') {
    gitCommit = 'unknown';
}

console.log(`Manual Version: ${manualVersion}`);
console.log(`Software Version: ${softwareVersion}`);
console.log(`Using git commit: ${gitCommit}`);

// 3. Write git_info.tex
const gitInfoContent = `
\\newcommand{\\gitcommit}{${gitCommit}}
\\newcommand{\\manualversion}{${manualVersion}}
\\newcommand{\\softwareversion}{${softwareVersion}}
`;
fs.writeFileSync(GIT_INFO_FILE, gitInfoContent);

// 4. Run lualatex
function runLuaLatex() {
    console.log('Running lualatex...');
    const result = spawnSync('lualatex', [
        '-interaction=nonstopmode',
        `-output-directory=${BUILD_DIR}`,
        'main.tex'
    ], { 
        cwd: DOCS_DIR,
        stdio: 'inherit' 
    });

    if (result.status !== 0) {
        console.error('lualatex failed.');
        process.exit(1);
    }
}

// Run twice for TOC and references
runLuaLatex();
runLuaLatex();

console.log(`Build complete. PDF available at: ${path.join(BUILD_DIR, 'main.pdf')}`);
