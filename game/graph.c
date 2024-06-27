#include "graph.h"
#include "game.h"
#include "engine/arraylist.h"
#include "engine/tex.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raymath.h>
#include <rlgl.h>

// https://github.com/raysan5/raylib/blob/f05316b11d0a1b17088e3e75e4682e2e7ce54b91/examples/textures/textures_polygon.c#L114
// DrawTexturePoly() - Could help drawing the graph texture.

extern struct game game;

Texture2D graphtex;

static float scr_border_left(void)
{
	const float zoom = game.camera.zoom;
	return game.camera.target.x / GRAPH_SCALE * zoom -
	       game.window->screen_w / 2;
}

static float scr_border_right(void)
{
	const float zoom = game.camera.zoom;
	return game.camera.target.x / GRAPH_SCALE * zoom +
	       game.window->screen_w / 2;
}

enum toktype {
	TT_DEFAULT,
	TT_UNKNOWN,
	TT_NUM,
	TT_X,
	TT_OP,
	TT_PAREN,
	TT_FUNC,
};

enum prec {
	PR_DEFAULT,
	PR_ADDSUB,
	PR_MULDIV,
	PR_EXP,
	PR_PAREN,
};

struct tok {
	enum toktype type;
	enum prec prec;
	union {
		float num;
		char op;
		char fname[8];
	};
};

struct node {
	_Bool negative;
	struct node *left, *right;
	struct tok tok;
};

struct lexer {
	const char *s;
	size_t i;
};

static enum toktype tt_from_char(char c)
{
	return c >= '0' && c <= '9' ? TT_NUM :
	       c == 'x'		    ? TT_X :
	       c >= 'a' && c <= 'z' ? TT_FUNC :
	       c == '+' || c == '-' || c == '*' || c == '/' || c == '^' ?
				      TT_OP :
	       c == '(' || c == ')' ? TT_PAREN :
				      TT_UNKNOWN;
}

static struct tok tok_from_buf(const char *buf, enum toktype tt)
{
	struct tok tok;
	tok.type = tt;

	switch (tt) {
	case TT_NUM:
		tok.num = (float)atof(buf);
		break;
	case TT_OP:
	case TT_PAREN:
		tok.op = *buf;
		break;
	case TT_FUNC:
		memcpy(tok.fname, buf, strlen(buf));
		break;
	default:
		break;
	}

	return tok;
}

static char lex_advance(struct lexer *lex)
{
	return lex->s[lex->i++];
}

static char lex_peek(struct lexer *lex, size_t by)
{
	return lex->s[lex->i + by - 1];
}

static size_t get_prec(struct tok tok)
{
	switch (tok.op) {
	case '+':
	case '-':
		return PR_ADDSUB;
	case '*':
	case '/':
		return PR_MULDIV;
	case '^':
		return PR_EXP;
	default:
		return PR_DEFAULT;
	}
}

static struct arraylist lex_expr(struct lexer *lex)
{
	struct arraylist toks =
		arraylist_create_preloaded(sizeof(struct tok), 32, 1);

	enum toktype search = TT_DEFAULT;

	char *buffer = calloc(64, 1);
	size_t bi = 0;

	_Bool first = 1;

	for (;;) {
		const char c = lex_advance(lex);

		const enum toktype tt = tt_from_char(c);

		first = 0;

		if (search == TT_DEFAULT) {
			search = tt;
			buffer[bi++] = c;
			first = 1;
		}

		const char next = lex_peek(lex, 1);
		const enum toktype next_tt = tt_from_char(next);

		if (!first)
			buffer[bi++] = c;

		if (next_tt != search ||
		    (next_tt != TT_NUM && next_tt != TT_FUNC)) {
			if (search != TT_UNKNOWN) {
				struct tok tok = tok_from_buf(buffer, search);
				arraylist_pushback(&toks, &tok);
			}

			search = TT_DEFAULT;
			memset(buffer, 0, bi);
			bi = 0;
		}

		if (next == 0)
			break;
	}

	free(buffer);

	return toks;
}

static struct arraylist lex(const char *s)
{
	struct lexer lex = { .s = s, .i = 0 };
	return lex_expr(&lex);
}

static struct node *newnode(struct tok tok)
{
	struct node *node = malloc(sizeof(struct node));
	node->tok = tok;
	node->negative = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static void node_destroy(struct node *node)
{
	if (node == NULL)
		return;

	node_destroy(node->left);
	node_destroy(node->right);
	free(node);
}

struct parser {
	struct arraylist tokens;
	size_t i;
};

static struct tok parser_advance(struct parser *par)
{
	struct tok *tok = arraylist_get(&par->tokens, par->i++);
	return *tok;
}

static struct tok parser_peek(struct parser *par, size_t by)
{
	struct tok *tok = arraylist_get(&par->tokens, par->i + by - 1);
	return *tok;
}

struct node *parse_expr(struct parser *par, enum prec min_prec);

static struct node *parse_primary(struct parser *par)
{
	struct tok tok = parser_advance(par);
	struct tok next = parser_peek(par, 1);

	if (tok.type == TT_FUNC && next.type == TT_PAREN && next.op == '(') {
		struct node *f = newnode(tok);
		parser_advance(par);
		f->left = parse_expr(par, PR_DEFAULT);
		parser_advance(par);
		return f;
	}

	if (tok.type == TT_PAREN && tok.op == '(') {
		struct node *node = parse_expr(par, PR_DEFAULT);
		parser_advance(par);
		return node;
	}

	struct tok prev = parser_peek(par, -1);
	const _Bool negative = prev.type == TT_OP && tok.type == TT_OP &&
			       tok.op == '-' &&
			       (next.type == TT_NUM || next.type == TT_X);

	if (negative) {
		tok = parser_advance(par);
		next = parser_peek(par, 1);
	}

	if (next.type != TT_X) {
		struct node *node = newnode(tok);
		node->negative = negative;
		return node;
	}

	struct node *mul = newnode((struct tok){ .type = TT_OP, .op = '*' });
	mul->left = newnode(tok);
	mul->left->negative = negative;
	mul->right = newnode(parser_advance(par));
	return mul;
}

struct node *parse_expr(struct parser *par, enum prec min_prec)
{
	struct node *left = parse_primary(par);

	size_t tok_count = arraylist_count(&par->tokens);
	struct tok tok_next;
	for (; par->i < tok_count;) {
		tok_next = parser_peek(par, 1);

		if ((tok_next.type == TT_PAREN && tok_next.op == ')'))
			break;

		struct tok op_tok = tok_next;
		enum prec op_prec = get_prec(op_tok);

		if (op_prec < min_prec)
			break;

		parser_advance(par);

		struct node *right = parse_expr(par, op_prec + 1);

		struct node *bin = newnode(op_tok);
		bin->left = left;
		bin->right = right;
		left = bin;
	}

	return left;
}

static struct node *parse(struct arraylist tokens)
{
	struct parser par = { .tokens = tokens, .i = 0 };
	return parse_expr(&par, PR_DEFAULT);
}

static void view_toklist(struct arraylist *toks)
{
	printf("token list: ");
	for (size_t i = 0; i < arraylist_count(toks); ++i) {
		struct tok *tok = arraylist_get(toks, i);
		printf("{ t: %d, v: %f (%c) } ", tok->type, tok->num, tok->op);
	}
	printf("\n");
}

static void rview_nodes(struct node *node, size_t depth)
{
	printf("([%lu] %d, %f (%c))", depth, node->tok.type, node->tok.num,
	       node->tok.op);
	if (node->left != NULL && node->right != NULL) {
		printf("{ L: ");
		rview_nodes(node->left, depth + 1);
		printf(", R: ");
		rview_nodes(node->right, depth + 1);
		printf(" }");
	}
}

static void view_nodes(struct node *root)
{
	rview_nodes(root, 0);
	printf("\n");
}

static float calculate_for_x(struct node *node, float x);

static float calc_func(struct node *node, float x)
{
	const float v = calculate_for_x(node->left, x);

	if (strcmp(node->tok.fname, "sin") == 0)
		return sinf(v);
	if (strcmp(node->tok.fname, "abs") == 0)
		return fabsf(x);

	return 0;
}

static float calculate_for_x(struct node *node, float x)
{
	const struct tok tok = node->tok;

	switch (node->tok.type) {
	case TT_OP: {
		const float left = calculate_for_x(node->left, x);
		const float right = calculate_for_x(node->right, x);

		if (tok.op == '+')
			return left + right;
		if (tok.op == '-')
			return left - right;
		if (tok.op == '*')
			return left * right;
		if (tok.op == '/')
			return left / right;
		if (tok.op == '^')
			return powf(left, right);

		return 0;
	}
	case TT_NUM:
		return node->negative ? -node->tok.num : node->tok.num;
	case TT_X:
		return node->negative ? -x : x;
	case TT_FUNC:
		return calc_func(node, x);
	default:
		return 0;
	}
}

static struct arraylist gen_gpoints(struct node *ast)
{
	const size_t n = 5000;

	struct arraylist points =
		arraylist_create_preloaded(sizeof(Vector2), n, 1);

	for (size_t i = 0; i < n; ++i) {
		const float x = (float)i - (float)n / 2;
		const float y = calculate_for_x(ast, x);
		const Vector2 p = { x, y };
		arraylist_pushback(&points, &p);
	}

	return points;
}

void build_fgraph(const char *expr)
{
	struct arraylist tokens = lex(expr);
	//view_toklist(&tokens);

	struct node *ast = parse(tokens);
	//view_nodes(ast);

	//const float x = 5;
	//const float y = calculate_for_x(ast, x);
	//printf("expr:\"%s\" with x:%f -> y:%f\n", expr, x, y);

	if (game.graph_points.data != NULL)
		arraylist_destroy(&game.graph_points);

	game.graph_points = gen_gpoints(ast);
}

static void draw_line(Vector2 a, Vector2 b, Color c)
{
	a.y = -a.y;
	b.y = -b.y;
	DrawLineV(a, b, c);
}

void render_graph(void)
{
	struct arraylist *points = &game.graph_points;

	const Vector2 *vpoints = points->data;
	if (vpoints == NULL)
		return;

	const float width = 5;

	_Bool tangent_set = 0;
	Vector2 prev = vpoints[0];
	Vector2 prev_tan = { 0 };
	float prev_v = 0;

	for (size_t i = 1; i < arraylist_count(points); ++i) {
		Vector2 point = vpoints[i];
		point.y = -point.y;

		// Vector from previous to current
		Vector2 delta = { point.x - prev.x, point.y - prev.y };

		// The right hand normal to the delta vector
		Vector2 normal =
			Vector2Normalize((Vector2){ -delta.y, delta.x });

		// The v texture coordinate of the segment (add up the length of all the segments so far)
		const float v = prev_v + Vector2Length(delta);

		// Make sure the start point has a normal
		if (!tangent_set) {
			prev_tan = normal;
			tangent_set = 1;
		}

		// Extend out the normals from the previous and current points to get the quad for this segment
		Vector2 prevPosNormal =
			Vector2Add(prev, Vector2Scale(prev_tan, width));
		Vector2 prevNegNormal =
			Vector2Add(prev, Vector2Scale(prev_tan, -width));

		Vector2 currentPosNormal =
			Vector2Add(point, Vector2Scale(normal, width));
		Vector2 currentNegNormal =
			Vector2Add(point, Vector2Scale(normal, -width));

		// Draw the segment as a quad
		rlSetTexture(graphtex.id);
		rlBegin(RL_QUADS);
		rlColor4ub(255, 255, 255, 255);
		rlNormal3f(0.0F, 0.0F, 1.0F);

		rlTexCoord2f(0, prev_v);
		rlVertex2f(prevNegNormal.x, prevNegNormal.y);

		rlTexCoord2f(1, prev_v);
		rlVertex2f(prevPosNormal.x, prevPosNormal.y);

		rlTexCoord2f(1, v);
		rlVertex2f(currentPosNormal.x, currentPosNormal.y);

		rlTexCoord2f(0, v);
		rlVertex2f(currentNegNormal.x, currentNegNormal.y);
		rlEnd();

		prev = point;
		prev_v = v;
		prev_tan = normal;
	}
}

void render_fgraph_old(float (*f)(float x), Color color)
{
	const float bl = scr_border_left();
	const float br = scr_border_right();
	const float zoom = game.camera.zoom;

	int px = (int)bl * GRAPH_SCALE;
	const int lim = (int)br * GRAPH_SCALE;

	Vector2 prev = {
		(float)px / zoom,
		-(float)f((float)px / zoom) * GRAPH_SCALE,
	};

	for (++px; px <= lim; ++px) {
		const float x = (float)px / GRAPH_SCALE / zoom;
		const float y = -f(x);

		const float cx = x * GRAPH_SCALE;
		const float cy = y * GRAPH_SCALE;

		// draw using rlgl vertices
		const Vector2 current = { cx, cy };
		DrawLineV(prev, current, color);

		prev.x = cx;
		prev.y = cy;
	}
}

void graph_init(void)
{
	texture_load(&graphtex, "res/img/graphline.png");
	SetTextureFilter(graphtex, TEXTURE_FILTER_BILINEAR);
}
