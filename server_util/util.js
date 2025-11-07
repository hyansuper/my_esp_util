q=(s,e)=>(e??document).querySelector(s);
qa=(s,e)=>[...(e??document).querySelectorAll(s)];
eid=i=>document.getElementById(i);
zip=(x,...y)=>x.map((v,i)=>[v,...y.map(j=>j[i])]);
fetch_j=s=>fetch(s).then(r=>r.json());
fetch_t=s=>fetch(s).then(r=>r.text());
_eval=s=>{try{return eval(`(${s})`)}catch{}};
b2i=b=>b?1:0;
is_num=s=>/^-?\d+$/.test(s);
is_str=s=>typeof(s)=='string';
is_arr=s=>Array.isArray(s);
ver_cmp=(x,y)=>zip(...[x,y].map(o=>o.split('.').map(x=>parseInt(x)))).reduce((o,[a,b])=>o||a-b,0);
partial=(f,...a)=>(...b)=>f(...a,...b);
_split=s=>x=>x.split(s);split=_split('/');
dash2Camel=i=>{var o=i.split('-');return o[0]+o.slice(1).map(x=>x[0].toUpperCase()+x.slice(1)).join('');}
sel_lang=(i,e)=>qa('[data-en]',e).forEach(x=>x.attr(x.hasAttribute('placeholder')?'placeholder':'innerHTML',x.dataset[dash2Camel(i)]));
attr=(o,a,v)=>{if(typeof(a)=='object')return Object.assign(o,a);if(v==undefined)return o[a];o[a]=v;return o;};
(function(){
	var add_meth=(cls,nf)=>Object.entries(nf).map(([n,f])=>{cls.prototype[n]=f});
	add_meth(HTMLElement,{'attr':function(s,v){return attr(this,s,v)},'css':function(s,v){if(typeof(s)=='object'){Object.entries(s).forEach(([k,v])=>{this.style[k]=v});return this};if(v==null)return this.style[s];this.style[s]=v;return this}});
	add_meth(String,{'dom':function(){document.write(this)}});
	add_meth(Object,{'map':function(f){return Object.fromEntries(Object.entries(this).map(f))},'merge':function(x){return Object.assign(this,x)},'str':function(){return JSON.stringify(this)},'has':this.hasOwnProperty,'get':function(s,f){return this.has(s)?this[s]:f},'pop':function(s,f){var r=this.get(s,f);delete this[s];return r;}});
	arr2tab=(hdr,arr)=>arr.map(a=>Object.fromEntries((is_str(hdr)?hdr.split('/'):hdr).map((k,i)=>[k,a[i]])));
	var no_tail_el=['input','br'];
	var binary_attr = ['selected', 'checked'];
	var special_prop=['html','{}','<>','[]'];
	var if_fn=(f,...d)=>typeof(f)=='function'?f(...d):f;
	var j2h_fn=(tmpl,data,index)=>{
		if(Array.isArray(tmpl))return tmpl.map(t=>j2h_fn(t,data)).join('');
		if(typeof(tmpl)=='object'){
			var ndata=tmpl.get('{}',o=>o)(data); // new data
			var idata=tmpl.get('[]',o=>ndata)(data); // inner data
			var html=tmpl.get('html','');
			var tag=if_fn(tmpl.get('<>',''),ndata);
			if(tag==='') return j2h_fn(html,idata);
			var attr_list=Object.entries(tmpl).filter(([k,v])=>!special_prop.includes(k)).map(([k,v])=>{v=if_fn(v,ndata,index);return binary_attr.includes(k)?(v?k:''):`${k}='${v}'`}).join(' ');
			return '<' + tag + (attr_list.length? (' '+attr_list): '') + (no_tail_el.includes(tag)? '/>': `>${j2h_fn(html,idata,index)}</${tag}>`);
		}
		return ''+if_fn(tmpl,data,index);
	};
	j2h=tmpl=>{
		var fn=data=>j2h_fn(tmpl,data);
		fn.batch=data=>data.map((d,i)=>j2h_fn(tmpl,d,i)).join('');
		return fn;
	};
	_id=o=>o.id;_txt=o=>o.text;_val=o=>o;_i=(o,i)=>i;_0=o=>o[0];_1=o=>o[1];_2=o=>o[2];_3=o=>o[3];
	ml=(tag,opt)=>j2h(({'<>':tag,'data-en':_0,'data-zh':x=>(x[1]??x[0])}).merge(opt));
	ml_div=j2h({'<>':'div',style:'text-align:right',html:['🌐',{'<>':'select',id:'_ml_sel',oninput:'sel_lang(this.value)',html:j2h({'<>':'option',value:_0,html:_1}).batch}]})([['en','English'],['zh','中文']]);
}());