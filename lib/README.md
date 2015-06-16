
* **node_module.js**: module system, inspired by Node.js.

* **util.js**: from Node.js, removed content related to deprecated interfaces.

    * dependency: none

* **path.js**: from Node.js.

    * dependency: `util`
    
* **events.js**: from Node.js.

	* dependency: `util`, sometimes `domain`
	
* **domain.js**: from Node.js.

	* dependency: `util`, sometimes `events`
