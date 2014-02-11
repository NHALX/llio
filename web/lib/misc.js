//Let's create the namespace
var NICOLA = {};
NICOLA.is_object = function(mixed_var) {
    if (Object.prototype.toString.call(mixed_var) === '[object Array]') {
        return false;
    }
    return mixed_var !== null && typeof mixed_var == 'object';
}
//This is the main merge function
NICOLA.merge = function(a, b) {
    //create a cache object that will be returned
    var cache = {};
    //call the NICOLA.unpackObject function which does the dirty work on both objects
    cache = NICOLA.unpackObject(a, cache);
    cache = NICOLA.unpackObject(b, cache);
    //return the merged objects
    return cache;
}

NICOLA.unpackObject = function(a, cache) {
    for (prop in a) {
        //filter out properties inherited from the prototype chain
        if (a.hasOwnProperty(prop)) {
            //if the cache objects doesn't contain the property, set it
            if (cache[prop] === undefined) {
                cache[prop] = a[prop];
            } else {
                //if the property is already set, check if the type of the colliding properties 
                //is the same
                if (typeof cache[prop] === typeof a[prop]) {
                    //if it is the same type, check if both preperties are objects
                    if (NICOLA.is_object(a[prop]) && NICOLA.is_object(cache[prop])) {
                        //if is an object, call recursively the NICOLA.merge to merge the object
                        cache[prop] = NICOLA.merge(cache[prop], a[prop]);
                    //check if the property is a number or a string
                    } else if (typeof cache[prop] === 'number' || typeof cache[prop] === 'string') {
                        //if it's a number, just sum the properties, if it's a string
                        //concatenate it
                        cache[prop] += a[prop];
                    } else {
                        //in all other cases, just use the newest value
                        cache[prop] = a[prop];
                    }
                }
            }
        }
    }
    return cache;
}

NICOLA.showobject = function(c) {
	var txt = [];
    for (prop in c) {
        if (c.hasOwnProperty(prop)) {
            if (NICOLA.is_object(c[prop])) {
                txt += NICOLA.showobject(c[prop]);
            } else {
				if (typeof c[prop] === 'number') {
					if (c[prop] != 0)
		                txt += prop + ':' + c[prop].toFixed(2) + '\n';
				} else {
				    txt += prop + ':'+typeof c[prop] + ':' + c[prop] + '\n';
				}
            }
        }
    }
	
	return txt;
}
/*
//this is a simple function to show all the properties of an object, recursively
NICOLA.printobject = function(c) {
    for (prop in c) {
        if (c.hasOwnProperty(prop)) {
            if (NICOLA.is_object(c[prop])) {
                 $('body').append('prop:[' + prop + '] is an object, we print the properties here<br />');
                NICOLA.printobject(c[prop]);
            } else {
                $('body').append('prop:[' + prop + '] - value:[' + c[prop] + ']<br/>');
            }
        }
    }
}

//some setup, this object will be merged
var a = {
    en: 5,
    fr: 3,
    in : 9,
    lang: "js",
    object: {
        nestedProp: 6
    }

}
var b = {
    en: 8,
    fr: 21,
    br: 8,
    lang: "en",
    object: {
        nestedProp: 1,
        unique: "myne"
    }
}
//merge the objects
var c = NICOLA.merge(a, b)
//print the results
NICOLA.printobject(c);
*/