import Vue from 'vue'
import Router from 'vue-router'
import Home from '@/components/Home.vue'
import ComputerUsage from '@/components/ComputerUsage.vue'
import Cell from '@/components/Cell.vue'
import Example from '@/components/Example.vue'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'Home',
      component: Home
    },{
      path: '/show/:id',
      name: 'ComputerUsage',
      component: ComputerUsage,
      props: true
    },{
      path: '/cell',
      name: 'Cell',
      component: Cell,
      props: true
    },{
      path: '/example',
      name: 'Example',
      component: Example,
      props: true
    }
  ]
})
